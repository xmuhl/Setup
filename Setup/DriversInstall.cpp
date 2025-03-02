#include "stdafx.h"
#include "DriversInstall.h"
#include "DebugStuff.h"
#include "UDefault.h"
#include "SetupDlg.h"
#include <Winspool.h>
#include "GetSysVersion.h"

#define nullptr NULL

extern CString g_MesTile;
extern CString g_Name;
bool g_bSetupProcEnd = false;				// 安装进程结束全局标识
bool g_bAutoApprove = false;				// 标识是否完成自动确认“安全警告”窗口
bool g_bFindWarnPopUp = false;				// 通过查找指定安全警告内容判断当前弹出窗口是否为无法验证警告窗口
HWND ghInstall = nullptr;					//  "安装"按钮句柄

////64位系统下要创建进程调用64位安装程序
//int X64OnlineInstallerProcess(void)
//{
//	int nRet = -1;
//	PROCESS_INFORMATION pi = { nullptr };
//
//	do
//	{
//		STARTUPINFO si = { sizeof(si) };
//		GetStartupInfo(&si);
//		TCHAR szProcessX64[] = _T("Setup_X64.exe");
//		if (!::CreateProcess(nullptr, szProcessX64, nullptr, nullptr, FALSE,
//			0, nullptr, nullptr, &si, &pi))
//		{
//			nRet = -3;
//			break;
//		}
//		::CloseHandle(pi.hThread);
//		pi.hThread = nullptr;
//
//		if (::WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_OBJECT_0)
//		{
//			nRet = -4;
//			break;
//		}
//		DWORD dwExitCode = 0;
//		if (!::GetExitCodeProcess(pi.hProcess, &dwExitCode))
//		{
//			nRet = -5;
//			break;
//		}
//		nRet = (int)dwExitCode;
//	} while (0);
//
//	if (pi.hThread != nullptr)
//	{
//		::CloseHandle(pi.hThread);
//		pi.hThread = nullptr;
//	}
//
//	if (pi.hProcess != nullptr)
//	{
//		::CloseHandle(pi.hProcess);
//		pi.hProcess = nullptr;
//	}
//
//	return nRet;
//}

// 后台自动安装打印机驱动线程
UINT InstallerDriverProc(LPVOID pParam)
{
	SetUp* structSetup = (SetUp*)pParam;

	BOOL bRet = FALSE;
	BOOL bX64 = Is64OS();

	if (structSetup->bOnlineSetup) //是否在线安装
	{
		DebugMesBox(_T("OnlineInstalled"));

		bRet = OnlineInstalled(structSetup->hWnd, structSetup->PrintModel, structSetup->InstallName, structSetup->Point, structSetup->GPD, bX64, structSetup->MFG, structSetup->bNewModel, structSetup->infName);

		// 先终止进度条    add by hl 2017.08.11
		SendMessage(structSetup->hWnd, UM_ENDPROGBAR, 0, 0);

		if (!bRet)
		{
			CString strInfoMes(LPCTSTR(IDS_OnlineInstallerFail));
			MesError(strInfoMes);

			//失败
			SendMessage(structSetup->hWnd, UM_ENDSTALL, 0, 0);

			g_bSetupProcEnd = true;
			delete pParam; //释放指针
			pParam = NULL;

			return 0;
		}
		else
		{
			//成功
			SendMessage(structSetup->hWnd, UM_ENDSTALL, 0, 1);
			g_bSetupProcEnd = true;
			delete pParam; //释放指针
			pParam = NULL;

			return 0;
		}
	}
	else
	{
		DebugMesBox(_T("PreInstalled"));

		int iRet = 0;
		iRet = PreInstalled(structSetup->hWnd, structSetup->SeriesName, structSetup->PrintModel, structSetup->InstallName, structSetup->Port, structSetup->GPD, structSetup->Point, bX64, structSetup->file, structSetup->infName);

		// 先终止进度条    add by hl 2017.08.11
		SendMessage(structSetup->hWnd, UM_ENDPROGBAR, 0, 0);

		if (iRet)
		{
			CString str;

			switch (iRet)
			{
			case  1:
				str = _T("错误原因：拷贝文件失败\r\n\r\n请关闭应用程序，重启操作系统再次尝试安装！\r\n\r\n如错误情况依然存在，请联系设备厂家获取进一步的技术支持。");
				break;
			case  2:
				str = _T("错误原因：安装inf文件失败\r\n\r\n请关闭应用程序，重启操作系统再次尝试安装！\r\n\r\n如错误情况依然存在，请联系设备厂家获取进一步的技术支持。");
				break;
			case  3:
				str = _T("错误原因：增加打印机驱动失败\r\n\r\n请下载最新版本驱动程序，并重启操作系统再次尝试安装！\r\n\r\n如错误情况依然存在，请联系设备厂家获取进一步的技术支持。");
				break;
			case  4:
				str = _T("错误原因：增加打印机失败\r\n\r\n请下载最新版本驱动程序，并重启操作系统再次尝试安装！\r\n\r\n如错误情况依然存在，请联系设备厂家获取进一步的技术支持。");
				break;
			case  5:
				str = _T("错误原因：写入注册表失败\r\n\r\n请关闭应用程序，重启操作系统再次尝试安装！\r\n\r\n如错误情况依然存在，请联系设备厂家获取进一步的技术支持。");
				break;
			case  6:
				str = _T("错误原因：设置默认打印机失败\r\n\r\n请下载最新版本驱动程序，并重启操作系统再次尝试安装！\r\n\r\n如错误情况依然存在，请联系设备厂家获取进一步的技术支持。");
				break;
			case  7:
				str = _T("错误原因：获取系统路径失败\r\n\r\n请下载最新版本驱动程序，并重启操作系统再次尝试安装！\r\n\r\n如错误情况依然存在，请联系设备厂家获取进一步的技术支持。");
				break;
			default:
				str = _T("未知错误，请联系设备厂家获取进一步的技术支持。");
				break;
			}

			MesError(str);

			//失败
			SendMessage(structSetup->hWnd, UM_ENDSTALL, 0, 0);
		}
		else
		{
			//成功
			SendMessage(structSetup->hWnd, UM_ENDSTALL, 0, 1);
		}

		g_bSetupProcEnd = true;

		//delete pParam; //释放指针
		//pParam = NULL;

		// 释放结构体指针
		delete structSetup;
		structSetup = nullptr;

		return 0;
	}
}

// 自动点击“Windows安全警告”弹窗的线程
UINT AutoApproveProc(LPVOID pParam)
{
	// 循环判断是否弹出“安全警告”窗口
	do
	{
		::EnumWindows(EnumWindowsProc, NULL);
		::Sleep(200);
	} while ((!g_bAutoApprove) && (!g_bSetupProcEnd));

	delete pParam; //释放指针

	return 0;
}

// 枚举弹出“Windows安装警告”窗口，找到单选框位置并自动勾选后，发送点击消息到“安装”按钮，实现自动确认关闭弹窗效果
BOOL __stdcall EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	TCHAR m_strTitle[MAX_PATH];
	if (::GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE)
	{
		::GetWindowText(hWnd, m_strTitle, MAX_PATH - 1);
		CString str = m_strTitle;
		//if (wcscmp(L"Windows 安全", m_strTitle) == 0)
		if (str.Find(_T("Windows 安全")) != -1)
		{
			::EnumChildWindows(hWnd, EnumChildProc, NULL);

			if (ghInstall != nullptr)
			{
				if (!g_bFindWarnPopUp)							// 发现已通过验证版本的安全警告弹出窗口
				{
					CRect rcRect;
					::GetWindowRect(ghInstall, rcRect);

					int width = ::GetSystemMetrics(SM_CXSCREEN);
					int height = ::GetSystemMetrics(SM_CYSCREEN);

					int X = rcRect.left - rcRect.Width() * 2;
					int Y = rcRect.top + rcRect.Height();

					INPUT input;
					memset(&input, 0, sizeof(INPUT));
					input.type = INPUT_MOUSE;
					input.mi.dx = X * 65536 / width;			// 坐标要通过转换，光标在屏幕中被分成65535个小块
					input.mi.dy = Y * 65536 / height;
					input.mi.mouseData = 0;
					input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
					input.mi.time = 0;
					input.mi.dwExtraInfo = 0;

					SendInput(1, &input, sizeof(INPUT));

					memset(&input, 0, sizeof(INPUT));
					input.type = INPUT_MOUSE;
					input.mi.dx = X * 65536 / width;
					input.mi.dy = Y * 65536 / height;
					input.mi.mouseData = 0;
					input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
					input.mi.time = 0;
					input.mi.dwExtraInfo = 0;

					SendInput(1, &input, sizeof(INPUT));

					memset(&input, 0, sizeof(INPUT));
					input.type = INPUT_MOUSE;
					input.mi.dx = X * 65536 / width;
					input.mi.dy = Y * 65536 / height;
					input.mi.mouseData = 0;
					input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
					input.mi.time = 0;
					input.mi.dwExtraInfo = 0;

					SendInput(1, &input, sizeof(INPUT));
				}

				::PostMessage(ghInstall, BM_CLICK, NULL, NULL);				// 发送消息点击安装按钮

				g_bAutoApprove = true;					// 自动确认Windows安全警告
			}
		}
	}
	return TRUE;
}

// 在“安全警告”弹出窗口中查找“安装”按钮并返还按钮句柄
BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam)
{
	TCHAR szWndTitle[1024];
	ZeroMemory(szWndTitle, 1024);

	int nLen = ::GetWindowText(hwndChild, szWndTitle, 1024);

	CString ctrlTitle = szWndTitle;

	if (wcscmp(szWndTitle, L"安装(&N)") == 0 || wcscmp(szWndTitle, L"安装(&I)") == 0)
	{
		ghInstall = hwndChild;
	}
	else if (wcscmp(szWndTitle, L"始终安装此驱动程序软件(&I)") == 0)
	{
		g_bFindWarnPopUp = true;

		ghInstall = hwndChild;
	}

	return TRUE;
}