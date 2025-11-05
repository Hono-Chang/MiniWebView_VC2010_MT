// MiniWebView.h
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

// Exports for the DLL (IE WebBrowser control host)
__declspec(dllexport) HWND __stdcall MV_Create(HWND Parent, int x, int y, int w, int h);
__declspec(dllexport) void __stdcall MV_LoadURL(HWND hBrowser, const char* url);
__declspec(dllexport) void __stdcall MV_LoadURLPost(HWND hBrowser, const unsigned char* postData, int postLen, const char* contentType, const char* url);
__declspec(dllexport) void __stdcall MV_ExecScript(HWND hBrowser, const char* script);
__declspec(dllexport) int  __stdcall MV_SetEmulationIE11(void);
__declspec(dllexport) void __stdcall MV_Destroy(HWND hBrowser);

#ifdef __cplusplus
}
#endif