/************************************************************************

FileName:	GetSysVersion.cpp

Copyright (c) 2009-2012

Writer:	黄磊

create Date:	120703

Version:		1.0

Main Content: 判断当前系统版本

rebuild:

/************************************************************************/

#include "stdafx.h"
#include "GetSysVersion.h"
#include "DebugStuff.h"

#pragma warning(disable: 4996)   //屏蔽GetVersionEx被判决的警告
typedef void (WINAPI* PGNSI)(LPSYSTEM_INFO);      // 用于动态加载GetNativeSystemInfo

// 获取操作系统版本信息(win98~win8)
DWORD  GetNtVersion(void)
{
	DWORD NtVersion = 1;   // 默认返回WinXp版本ID

	OSVERSIONINFO osver = { 0 };
	osver.dwOSVersionInfoSize = sizeof(osver);
	GetVersionEx(&osver);
	if (osver.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		BOOL bIs64b = Is64OS();				// 判断是否为64位操作系统   add by hl 2011.11.09

		if (osver.dwMajorVersion > 5)     // Windows Server 2008 or Windows Vista. Win7, Win8, Win8.1
		{
			if (bIs64b)
			{
				NtVersion = 4;
			}
			else
			{
				NtVersion = 3;
			}
		}
		else      // Windows Server 2003 R2, Windows Server 2003, Windows XP, or Windows 2000.
		{
			if (bIs64b)
			{
				NtVersion = 2;
			}
			else
			{
				NtVersion = 1;
			}
		}
	}
	else
	{
		if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			NtVersion = 0;			// Win 98/Win95
		}
	}

	return NtVersion;
}

//// 判断操作系统位数（x86/x64)
//BOOL Is64OS(void)
//{
//	BOOL bIs64OS = FALSE;
//
//	// 判断当前操作系统位数									add by hl 2011.8.30
//	SYSTEM_INFO si;
//
//	PGNSI pGNSI;
//	// 采用动态加载GetNativeSystemInfo以解决winxp以下系统不支持该函数的问题。add by hl 2011.09.21
//	pGNSI = (PGNSI)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetNativeSystemInfo");
//	if (NULL != pGNSI)
//	{
//		pGNSI(&si);
//	}
//	else
//	{
//		GetSystemInfo(&si);
//	}
//
//	if ((si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ||
//		(si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64))
//	{
//		bIs64OS = TRUE;    // x64 for Win7/Vista/WinServer/Winxp
//	}
//	else
//	{
//	}
//	return bIs64OS;
//}

// 判断当前系统是否为 64 位
BOOL Is64OS()
{
#if defined(_WIN64)
	return TRUE;  // 64 位程序肯定运行在 64 位 Windows 上
#elif defined(_WIN32)
	BOOL bIsWow64 = FALSE;
	typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process;

	// 获取 IsWow64Process 函数地址
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			// 调用失败，假设为非 64 位系统
			bIsWow64 = FALSE;
		}
	}
	return bIsWow64;
#else
	return FALSE; // 非 Windows 系统
#endif
}
