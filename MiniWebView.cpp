// MiniWebView.cpp
// Target: Visual C++ 2010, /MT static CRT
// Uses ATL CAxWindow to host the IE WebBrowser control (Shell.Explorer.2)

#include "MiniWebView.h"
#include <windows.h>
#include <atlbase.h>
#include <atlwin.h>
#include <atlhost.h>
#include <exdisp.h>
#include <mshtml.h>
#include <oleauto.h>
#include <comdef.h>
#include <map>
#include <mutex>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

using namespace ATL;

static std::map<HWND, CAxWindow*> g_map;
static std::mutex g_map_mutex;

static BSTR Utf8ToBSTR(const char* utf8)
{
    if(!utf8) return SysAllocString(L"");
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    BSTR b = SysAllocStringLen(nullptr, wlen);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, b, wlen);
    return b;
}

extern "C" HWND __stdcall MV_Create(HWND Parent, int x, int y, int w, int h)
{
    if(!Parent) return NULL;
    CoInitialize(NULL);
    CAxWindow* pAx = new CAxWindow();
    RECT rc = { x, y, x + w, y + h };
    if(!pAx->Create(Parent, rc, nullptr, WS_CHILD | WS_VISIBLE))
    {
        delete pAx;
        return NULL;
    }
    CComPtr<IDispatch> spDisp;
    HRESULT hr = AtlAxCreateControl(CComBSTR(L"Shell.Explorer.2"), pAx->m_hWnd, &spDisp);
    if(FAILED(hr))
    {
        pAx->DestroyWindow();
        delete pAx;
        return NULL;
    }
    {
        std::lock_guard<std::mutex> lk(g_map_mutex);
        g_map[pAx->m_hWnd] = pAx;
    }
    return pAx->m_hWnd;
}

extern "C" void __stdcall MV_LoadURL(HWND hBrowser, const char* url)
{
    if(!hBrowser || !url) return;
    std::lock_guard<std::mutex> lk(g_map_mutex);
    auto it = g_map.find(hBrowser);
    if(it==g_map.end()) return;
    CAxWindow* pAx = it->second;
    CComPtr<IDispatch> spDisp;
    if(FAILED(AtlAxGetControl(pAx->m_hWnd, &spDisp)) || !spDisp) return;
    CComPtr<IWebBrowser2> spWB;
    spDisp->QueryInterface(IID_IWebBrowser2, (void**)&spWB);
    if(!spWB) return;
    CComVariant vEmpty;
    CComBSTR bstrUrl = Utf8ToBSTR(url);
    spWB->Navigate(bstrUrl, &vEmpty, &vEmpty, &vEmpty, &vEmpty);
    SysFreeString(bstrUrl);
}

static SAFEARRAY* CreateSafeArrayFromBytes(const unsigned char* data, int len)
{
    if(!data || len<=0) return nullptr;
    SAFEARRAY* psa = SafeArrayCreateVector(VT_UI1, 0, len);
    if(!psa) return nullptr;
    void* pv;
    SafeArrayAccessData(psa, &pv);
    memcpy(pv, data, len);
    SafeArrayUnaccessData(psa);
    return psa;
}

extern "C" void __stdcall MV_LoadURLPost(HWND hBrowser, const unsigned char* postData, int postLen, const char* contentType, const char* url)
{
    if(!hBrowser || !url) return;
    std::lock_guard<std::mutex> lk(g_map_mutex);
    auto it = g_map.find(hBrowser);
    if(it==g_map.end()) return;
    CAxWindow* pAx = it->second;
    CComPtr<IDispatch> spDisp;
    if(FAILED(AtlAxGetControl(pAx->m_hWnd, &spDisp)) || !spDisp) return;
    CComPtr<IWebBrowser2> spWB;
    spDisp->QueryInterface(IID_IWebBrowser2, (void**)&spWB);
    if(!spWB) return;

    CComBSTR bstrUrl = Utf8ToBSTR(url);
    VARIANT vFlags; VariantInit(&vFlags);
    VARIANT vTargetFrame; VariantInit(&vTargetFrame);
    VARIANT vPostData; VariantInit(&vPostData);
    VARIANT vHeaders; VariantInit(&vHeaders);

    SAFEARRAY* psa = CreateSafeArrayFromBytes(postData, postLen);
    if(psa) { vPostData.vt = VT_ARRAY | VT_UI1; vPostData.parray = psa; }

    if(contentType && strlen(contentType)>0) {
        char headerBuf[512];
        sprintf_s(headerBuf, sizeof(headerBuf), "Content-Type: %s\r\n", contentType);
        BSTR bh = Utf8ToBSTR(headerBuf);
        vHeaders.vt = VT_BSTR; vHeaders.bstrVal = bh;
    }

    VARIANT varUrl; VariantInit(&varUrl);
    varUrl.vt = VT_BSTR; varUrl.bstrVal = bstrUrl;

    spWB->Navigate2(&varUrl, &vFlags, &vTargetFrame, &vPostData, &vHeaders);

    VariantClear(&vPostData);
    VariantClear(&vHeaders);
    VariantClear(&varUrl);
    SysFreeString(bstrUrl);
}

extern "C" void __stdcall MV_ExecScript(HWND hBrowser, const char* script)
{
    if(!hBrowser || !script) return;
    std::lock_guard<std::mutex> lk(g_map_mutex);
    auto it = g_map.find(hBrowser);
    if(it==g_map.end()) return;
    CAxWindow* pAx = it->second;
    CComPtr<IDispatch> spDisp;
    if(FAILED(AtlAxGetControl(pAx->m_hWnd, &spDisp)) || !spDisp) return;
    CComPtr<IWebBrowser2> spWB;
    spDisp->QueryInterface(IID_IWebBrowser2, (void**)&spWB);
    if(!spWB) return;

    CComPtr<IDispatch> pDocDisp;
    if(FAILED(spWB->get_Document(&pDocDisp)) || !pDocDisp) return;
    CComPtr<IHTMLDocument2> pDoc2;
    if(FAILED(pDocDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc2)) || !pDoc2) return;
    CComPtr<IHTMLWindow2> pWin;
    if(FAILED(pDoc2->get_parentWindow(&pWin)) || !pWin) return;

    BSTR bScript = Utf8ToBSTR(script);
    CComBSTR lang(L"JavaScript");
    VARIANT vRet; VariantInit(&vRet);
    pWin->execScript(bScript, lang, &vRet);
    VariantClear(&vRet);
    SysFreeString(bScript);
}

extern "C" int __stdcall MV_SetEmulationIE11(void)
{
    wchar_t exePath[MAX_PATH]; GetModuleFileNameW(NULL, exePath, MAX_PATH);
    wchar_t* fileName = wcsrchr(exePath, '\\\\');
    if(fileName) fileName++; else fileName = exePath;
    HKEY hKey;
    LONG lr = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\\\Microsoft\\\\Internet Explorer\\\\Main\\\\FeatureControl\\\\FEATURE_BROWSER_EMULATION",
                              0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);
    if(lr != ERROR_SUCCESS) return 0;
    DWORD mode = 11001; // IE11 Edge mode
    lr = RegSetValueExW(hKey, fileName, 0, REG_DWORD, (const BYTE*)&mode, sizeof(mode));
    RegCloseKey(hKey);
    return (lr==ERROR_SUCCESS)?1:0;
}

extern "C" void __stdcall MV_Destroy(HWND hBrowser)
{
    if(!hBrowser) return;
    std::lock_guard<std::mutex> lk(g_map_mutex);
    auto it = g_map.find(hBrowser);
    if(it==g_map.end()) return;
    CAxWindow* pAx = it->second;
    g_map.erase(it);
    if(pAx) { pAx->DestroyWindow(); delete pAx; }
    CoUninitialize();
}