// Setup.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif
//#include "..\UIChina\resource.h"		// 主符号
#include "resource.h"		// 主符号

// CSetupApp:
// 有关此类的实现，请参阅 Setup.cpp
//

class CSetupApp : public CWinApp
{
public:
	CSetupApp();

	// 重写
public:
	virtual BOOL InitInstance();

	// 实现
	HINSTANCE mDllInstance;//存放动态库实例句柄
	DECLARE_MESSAGE_MAP()
};

extern CSetupApp theApp;
//UINT CloseService(LPVOID lParam);