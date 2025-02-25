#pragma once
#include "resource.h"

UINT InstallerDriverProc(LPVOID pParam);		   // 后台自动安装打印机驱动线程
UINT AutoApproveProc(LPVOID pParam);		   // 自动点击“Windows安全警告”弹窗的线程

BOOL __stdcall EnumWindowsProc(HWND hWnd, LPARAM lParam);
BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam);