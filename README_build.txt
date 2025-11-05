MiniWebView VC++2010 /MT Package
Contents:
- MiniWebView.h
- MiniWebView.cpp
- build_vs2010.bat   : build script for Visual Studio 2010 x86 (uses vcvarsall.bat)
- MiniWebView_VS2010.vcproj : VS2010 project skeleton (Win32 DLL)
- BCB6_Demo/Unit1.cpp  : BCB6 demo unit showing how to call the DLL from a form with Panel1
- README_build.txt : instructions to build with VS2010

Important: This environment cannot compile native Visual C++ binaries for you. The package contains source and build scripts so you can compile locally with Visual Studio 2010.

Build steps (on your machine with Visual Studio 2010 installed):
1) Open \"Visual Studio x86 Win32 Command Prompt\" or run:
   \"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\vcvarsall.bat\" x86
2) Run: build_vs2010.bat
   It will call cl with /MT and produce MiniWebView.dll and MiniWebView.lib in the folder.

Notes:
- Ensure ATL is installed in your VS2010 (Common Tools -> Visual C++ -> ATL support).
- If using the vcproj, open it in VS2010 and build Release|Win32 with Runtime Library set to Multi-threaded (/MT).