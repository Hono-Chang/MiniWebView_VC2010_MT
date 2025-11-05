// BCB6_Demo Unit1.cpp (same as provided earlier)
#include <vcl.h>
#pragma hdrstop
#include "Unit1.h"
#pragma package(smart_init)
#pragma resource "*.dfm"

TForm1 *Form1;

typedef HWND (__stdcall *FnCreate)(HWND,int,int,int,int);
typedef void (__stdcall *FnLoadURL)(HWND,const char*);
typedef void (__stdcall *FnLoadURLPost)(HWND,const unsigned char*,int,const char*,const char*);
typedef void (__stdcall *FnExecScript)(HWND,const char*);
typedef int  (__stdcall *FnSetEmu)(void);
typedef void (__stdcall *FnDestroy)(HWND);

HMODULE hDLL = NULL;
FnCreate pCreate = NULL; FnLoadURL pLoad = NULL; FnLoadURLPost pPost = NULL; FnExecScript pExec = NULL; FnSetEmu pSet = NULL; FnDestroy pDestroy = NULL;
HWND hBrowser = NULL;

void __fastcall TForm1::FormCreate(TObject *Sender)
{
    hDLL = LoadLibrary("MiniWebView.dll");
    if(!hDLL) { ShowMessage("LoadLibrary failed"); return; }
    pCreate = (FnCreate)GetProcAddress(hDLL, "MV_Create");
    pLoad   = (FnLoadURL)GetProcAddress(hDLL, "MV_LoadURL");
    pPost   = (FnLoadURLPost)GetProcAddress(hDLL, "MV_LoadURLPost");
    pExec   = (FnExecScript)GetProcAddress(hDLL, "MV_ExecScript");
    pSet    = (FnSetEmu)GetProcAddress(hDLL, "MV_SetEmulationIE11");
    pDestroy= (FnDestroy)GetProcAddress(hDLL, "MV_Destroy");

    if(pSet) pSet();

    if(pCreate) {
        hBrowser = pCreate(Panel1->Handle, 0, 0, Panel1->Width, Panel1->Height);
        if(!hBrowser) ShowMessage("Create failed");
    }

    if(pLoad && hBrowser) pLoad(hBrowser, "https://www.google.com");
}

void __fastcall TForm1::FormResize(TObject *Sender)
{
    if(hBrowser) SetWindowPos(hBrowser, NULL, 0,0, Panel1->Width, Panel1->Height, SWP_NOZORDER);
}

void __fastcall TForm1::ButtonPostClick(TObject *Sender)
{
    if(pPost && hBrowser) {
        const char* data = "a=1&b=2";
        pPost(hBrowser, (const unsigned char*)data, strlen(data), "application/x-www-form-urlencoded", "https://httpbin.org/post");
    }
}

void __fastcall TForm1::ButtonJSClick(TObject *Sender)
{
    if(pExec && hBrowser) pExec(hBrowser, "alert('Hello from DLL');");
}

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
    if(pDestroy && hBrowser) pDestroy(hBrowser);
    if(hDLL) FreeLibrary(hDLL);
}