// DriverSetupHelper.cpp: implementation of the CDriverSetupHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <tchar.h>

#include "DriverSetupHelper.h"
#include "GetSysVersion.h"

#include <Afxwin.h>
#include "DebugStuff.h"

//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[MAX_PATH] = __FILE__;
//#define new DEBUG_NEW
//#endif
//
//#ifndef DT
//#define DT	DebugTrace
//extern BOOL DebugTrace(const char* lpszFormat, ...);
//#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDriverSetupHelper::CDriverSetupHelper()
{
}

CDriverSetupHelper::~CDriverSetupHelper()
{
}

// //修订后的 Install 函数
//BOOL CDriverSetupHelper::Install(IN CString szInfFile, IN CString szHardwareID)
//{
//	//DebugMesBox(_T("Install"));
//	WIN32_FIND_DATA FindFileData;
//
//	BOOL RebootRequired = 0; // 必须清零
//
//	if (FindFirstFile(szInfFile, &FindFileData) == INVALID_HANDLE_VALUE)
//	{
//		GetErrAndLog(_T("FindFirstFile"));
//		return FALSE;
//	}
//
//	// 查看是否该设备已经存在
//	if (FindExistingDevice(szHardwareID))
//	{
//		// 无需创建设备节点，只需调用我们的 API
//
//		// 动态加载 NewDev.dll 中的 UpdateDriverForPlugAndPlayDevices 函数
//		HMODULE hNewDev = LoadLibrary(TEXT("newdev.dll"));
//		if (hNewDev == NULL)
//		{
//			GetErrAndLog(_T("无法加载 newdev.dll"));
//			return FALSE;
//		}
//
//		typedef BOOL(WINAPI* PFN_UpdateDriverForPlugAndPlayDevices)(
//			HWND hwndParent,
//			LPCTSTR HardwareId,
//			LPCTSTR FullInfPath,
//			DWORD InstallFlags,
//			PBOOL bRebootRequired OPTIONAL
//			);
//
//		PFN_UpdateDriverForPlugAndPlayDevices pUpdateDriverForPlugAndPlayDevices =
//			(PFN_UpdateDriverForPlugAndPlayDevices)GetProcAddress(hNewDev, "UpdateDriverForPlugAndPlayDevicesW");
//
//		if (!pUpdateDriverForPlugAndPlayDevices)
//		{
//			GetErrAndLog(_T("无法获取函数 UpdateDriverForPlugAndPlayDevices 的地址"));
//			FreeLibrary(hNewDev);
//			return FALSE;
//		}
//
//		if (!pUpdateDriverForPlugAndPlayDevices(
//			NULL,           // 无窗口句柄
//			szHardwareID,   // 硬件 ID
//			szInfFile,      // INF 文件路径
//			INSTALLFLAG_FORCE,
//			&RebootRequired))
//		{
//			GetErrAndLog(_T("UpdateDriverForPlugAndPlayDevices"));
//			FreeLibrary(hNewDev);
//			return FALSE; // 安装失败
//		}
//
//		// 释放动态库
//		FreeLibrary(hNewDev);
//	}
//
//	return TRUE; // 安装成功，无需重启
//}

 //修订后的 FindExistingDevice 函数
BOOL CDriverSetupHelper::FindExistingDevice(CString szHardwareID)
{
	HDEVINFO DeviceInfoSet;
	SP_DEVINFO_DATA DeviceInfoData = { 0 };
	DWORD i, err;
	BOOL Found;

	// 动态加载 SetupAPI.dll
	HMODULE hSetupAPI = LoadLibrary(TEXT("SetupAPI.dll"));
	if (hSetupAPI == NULL)
	{
		GetErrAndLog(_T("无法加载 SetupAPI.dll"));
		return FALSE;
	}

	// 定义函数指针类型
	typedef HDEVINFO(WINAPI* PFN_SetupDiGetClassDevs)(
		const GUID* ClassGuid,
		LPCTSTR Enumerator,
		HWND hwndParent,
		DWORD Flags
		);

	typedef BOOL(WINAPI* PFN_SetupDiEnumDeviceInfo)(
		HDEVINFO DeviceInfoSet,
		DWORD MemberIndex,
		PSP_DEVINFO_DATA DeviceInfoData
		);

	typedef BOOL(WINAPI* PFN_SetupDiGetDeviceRegistryProperty)(
		HDEVINFO DeviceInfoSet,
		PSP_DEVINFO_DATA DeviceInfoData,
		DWORD Property,
		PDWORD PropertyRegDataType,
		PBYTE PropertyBuffer,
		DWORD PropertyBufferSize,
		PDWORD RequiredSize
		);

	typedef BOOL(WINAPI* PFN_SetupDiDestroyDeviceInfoList)(
		HDEVINFO DeviceInfoSet
		);

	// 获取函数地址
	PFN_SetupDiGetClassDevs pSetupDiGetClassDevs =
		(PFN_SetupDiGetClassDevs)GetProcAddress(hSetupAPI, "SetupDiGetClassDevsW");
	PFN_SetupDiEnumDeviceInfo pSetupDiEnumDeviceInfo =
		(PFN_SetupDiEnumDeviceInfo)GetProcAddress(hSetupAPI, "SetupDiEnumDeviceInfo");
	PFN_SetupDiGetDeviceRegistryProperty pSetupDiGetDeviceRegistryProperty =
		(PFN_SetupDiGetDeviceRegistryProperty)GetProcAddress(hSetupAPI, "SetupDiGetDeviceRegistryPropertyW");
	PFN_SetupDiDestroyDeviceInfoList pSetupDiDestroyDeviceInfoList =
		(PFN_SetupDiDestroyDeviceInfoList)GetProcAddress(hSetupAPI, "SetupDiDestroyDeviceInfoList");

	// 检查函数是否成功获取
	if (!pSetupDiGetClassDevs || !pSetupDiEnumDeviceInfo ||
		!pSetupDiGetDeviceRegistryProperty || !pSetupDiDestroyDeviceInfoList)
	{
		GetErrAndLog(_T("无法获取 SetupAPI 所需的函数地址"));
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	//
	// 创建包含所有当前存在设备的设备信息集合
	//
	DeviceInfoSet = pSetupDiGetClassDevs(
		NULL, // 所有设备
		NULL,
		NULL,
		DIGCF_ALLCLASSES | DIGCF_PRESENT); // 系统所有设备
	if (DeviceInfoSet == INVALID_HANDLE_VALUE)
	{
		GetErrAndLog(_T("SetupDiGetClassDevs"));
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	//
	// 枚举所有设备
	//

	Found = FALSE;
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	for (i = 0; pSetupDiEnumDeviceInfo(DeviceInfoSet, i, &DeviceInfoData); i++)
	{
		DWORD DataT;
		LPTSTR p, buffer = NULL;
		DWORD buffersize = 0;

		//
		// 我们不知道缓冲区的大小，直到我们调用这个函数。
		// 因此，开始时传递 NULL，然后使用返回的所需缓冲区大小进行分配。
		//

		while (!pSetupDiGetDeviceRegistryProperty(
			DeviceInfoSet,
			&DeviceInfoData,
			SPDRP_HARDWAREID,
			&DataT,
			(PBYTE)buffer,
			buffersize,
			&buffersize))
		{
			if (GetLastError() == ERROR_INVALID_DATA) // 数据无效
			{
				//
				// 可能是没有硬件 ID 的旧设备，继续
				//
				break;
			}
			else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)  // 缓冲区不足
			{
				if (buffer)
					delete[] buffer;
				buffer = new TCHAR[buffersize / sizeof(TCHAR)];
				ZeroMemory(buffer, buffersize);
			}
			else
			{
				//
				// 未知错误
				//
				GetErrAndLog(_T("SetupDiGetDeviceRegistryProperty"));
				goto cleanup_DeviceInfo;
			}
		}

		if (GetLastError() == ERROR_INVALID_DATA)  // 数据无效
			continue;

		//
		// 比较缓冲区中的每个多字符串项与我们的硬件 ID
		//
		for (p = buffer; *p && (p < &buffer[buffersize / sizeof(TCHAR)]); p += lstrlen(p) + 1)
		{
			if (!_tcscmp(szHardwareID, p))
			{
				//DT(TEXT("Found! [%s]\n"),p);
				DebugMesBox(szHardwareID);
				Found = TRUE;
				break;
			}
		}

		if (buffer)
		{
			delete[] buffer;
			buffer = NULL;
		}

		if (Found)
		{
			break;
		}
	}

	if (GetLastError() != NO_ERROR && GetLastError() != ERROR_NO_MORE_ITEMS)
	{
		GetErrAndLog(_T("SetupDiEnumDeviceInfo"));
	}

cleanup_DeviceInfo:

	err = GetLastError();
	pSetupDiDestroyDeviceInfoList(DeviceInfoSet); // 清空枚举后的设备信息列表
	FreeLibrary(hSetupAPI); // 释放 SetupAPI.dll
	SetLastError(err);

	return Found;
}

// 安装驱动程序的函数
BOOL CDriverSetupHelper::Install(IN CString szInfFile, IN CString szHardwareID)
{
	//DebugMesBox(_T("Install"));
	WIN32_FIND_DATA FindFileData;

	BOOL RebootRequired = FALSE; // 必须初始化为 FALSE

	if (FindFirstFile(szInfFile, &FindFileData) == INVALID_HANDLE_VALUE)
	{
		GetErrAndLog(_T("FindFirstFile"));
		return FALSE;
	}

	// 查看是否该设备已经存在
	if (FindExistingDevice(szHardwareID))
	{
		// 检查当前系统是否为 64 位系统
		BOOL bX64 = Is64OS();

		// 如果是 64 位系统，创建进程调用 64 位安装程序
		if (bX64)
		{
			// 获取当前程序目录
			TCHAR szModulePath[MAX_PATH] = { 0 };
			GetModuleFileName(NULL, szModulePath, MAX_PATH);
			CString strModulePath = szModulePath;
			int nPos = strModulePath.ReverseFind('\\');
			CString strDir = strModulePath.Left(nPos + 1);

			// 构建 64 位辅助程序的路径
			CString strHelperPath = strDir + _T("DriverInstaller64.exe");

			// 检查辅助程序是否存在
			if (GetFileAttributes(strHelperPath) == INVALID_FILE_ATTRIBUTES)
			{
				GetErrAndLog(_T("无法找到 64 位辅助程序 DriverInstaller64.exe"));
				return FALSE;
			}

			// 构建命令行参数
			CString strCmdLine;
			strCmdLine.Format(_T("\"%s\" \"%s\" \"%s\""), (LPCTSTR)strHelperPath, (LPCTSTR)szInfFile, (LPCTSTR)szHardwareID);

			// 设置启动信息
			STARTUPINFO si = { sizeof(si) };
			PROCESS_INFORMATION pi = { 0 };
			si.dwFlags = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE; // 隐藏控制台窗口

			// 创建进程
			BOOL bSuccess = CreateProcess(
				NULL,                   // 应用程序名称
				strCmdLine.GetBuffer(), // 命令行
				NULL,                   // 进程安全属性
				NULL,                   // 线程安全属性
				FALSE,                  // 是否继承句柄
				0,                      // 创建标志
				NULL,                   // 环境变量
				NULL,                   // 当前目录
				&si,                    // 启动信息
				&pi                     // 进程信息
			);

			strCmdLine.ReleaseBuffer();

			if (!bSuccess)
			{
				GetErrAndLog(_T("无法启动 64 位辅助程序"));
				return FALSE;
			}

			// 等待辅助程序完成
			WaitForSingleObject(pi.hProcess, INFINITE);

			// 获取辅助程序的退出代码
			DWORD dwExitCode = 0;
			if (!GetExitCodeProcess(pi.hProcess, &dwExitCode))
			{
				GetErrAndLog(_T("获取辅助程序退出代码失败"));
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				return FALSE;
			}

			// 关闭句柄
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);

			if (dwExitCode != 0)
			{
				// 辅助程序返回错误
				CString strError;
				strError.Format(_T("辅助程序安装驱动程序失败，退出代码：%d"), dwExitCode);
				GetErrAndLog(strError);
				return FALSE;
			}

			// 如果辅助程序安装成功，检查是否需要重启（可以通过文件或其他方式传递）
			// 此处假设不需要处理重启
		}
		else
		{
			// 在 32 位系统上，直接调用 UpdateDriverForPlugAndPlayDevices
			if (!UpdateDriverForPlugAndPlayDevices(
				NULL,           // 无窗口句柄
				szHardwareID,   // 硬件 ID
				szInfFile,      // INF 文件路径
				INSTALLFLAG_FORCE,
				&RebootRequired))
			{
				GetErrAndLog(_T("UpdateDriverForPlugAndPlayDevices"));

				return FALSE; // 安装失败
			}
		}
	}

	return TRUE; // 安装成功
}

//install a device by a inf file and a szHardwareID
//set bMultiInstance to TURE to permit multiple device instance.
// BOOL CDriverSetupHelper::Install(IN CString szInfFile,IN CString szHardwareID,IN BOOL bMultiInstance OPTIONAL,OUT PBOOL pbRebootRequired OPTIONAL)
//BOOL CDriverSetupHelper::Install(IN CString szInfFile, IN CString szHardwareID)
//{
//	//DebugMesBox(_T("Install"));
//	WIN32_FIND_DATA FindFileData;
//
//	BOOL RebootRequired = 0; // Must be cleared.
//
//	if (FindFirstFile(szInfFile, &FindFileData) == INVALID_HANDLE_VALUE)
//	{
//		GetErrAndLog(_T("FindFirstFile"));
//		return FALSE;
//	}
//
//	// 查看是否该设备已经存在
//	if (FindExistingDevice(szHardwareID))
//	{
//		//
//		// No Need to Create a Device Node, just call our API.
//		//
//		if (!UpdateDriverForPlugAndPlayDevices(0, // No Window Handle
//			szHardwareID, // Hardware ID
//			szInfFile, // FileName
//			INSTALLFLAG_FORCE,
//			&RebootRequired))
//		{
//			GetErrAndLog(_T("UpdateDriverForPlugAndPlayDevices"));
//
//			return FALSE; // Install Failure
//		}
//	}
//
//	return TRUE; // Install Success, no reboot required.
//}

////是否存在szHardwareID为szszHardwareID的设备
//BOOL CDriverSetupHelper::FindExistingDevice(CString szHardwareID)
//{
//	HDEVINFO DeviceInfoSet;
//	SP_DEVINFO_DATA DeviceInfoData;
//	DWORD i, err;
//	BOOL Found;
//
//	//
//	// Create a Device Information Set with all present devices.
//	//
//	DeviceInfoSet = SetupDiGetClassDevs(NULL, // 所有设备
//		0,
//		0,
//
//		DIGCF_ALLCLASSES | DIGCF_PRESENT); // 系统所有设备
//	if (DeviceInfoSet == INVALID_HANDLE_VALUE)
//	{
//		GetErrAndLog(_T("SetupDiGetClassDevs"));
//		return false;
//	}
//
//	//
//	//  Enumerate through all Devices.
//	//
//
//	Found = FALSE;
//	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
//
//	for (i = 0; SetupDiEnumDeviceInfo(DeviceInfoSet, i, &DeviceInfoData); i++)
//	{
//		DWORD DataT;
//		LPTSTR p, buffer = NULL;
//		DWORD buffersize = 0;
//
//		//
//	   ////我们不知道硬件缓冲区大小，直到我们调用
//		//这个函数。因此，与空，开始与调用它，然后
//		//使用所需的缓冲区大小的Alloc必要的空间。
//		//调用保持我们的成功还是一个未知数失败。
//
//		while (!SetupDiGetDeviceRegistryProperty(
//			DeviceInfoSet,
//			&DeviceInfoData,
//			SPDRP_HARDWAREID,
//			&DataT,
//			(PBYTE)buffer,
//			buffersize,
//			&buffersize))
//		{
//			if (GetLastError() == ERROR_INVALID_DATA) //数据无效
//			{
//				//
//				// May be a Legacy Device with no szHardwareID. Continue.
//				//
//				break;
//			}
//			else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)  //传递到系统调用的数据区太小
//			{
//				buffer = new TCHAR[buffersize];
//				ZeroMemory(buffer, buffersize);
//			}
//			else
//			{
//				//
//				// Unknown Failure.
//				//
//				GetErrAndLog(_T("GetDeviceRegistryProperty"));
//				goto cleanup_DeviceInfo;
//			}
//		}
//
//		if (GetLastError() == ERROR_INVALID_DATA)  //数据无效
//			continue;
//
//		//
//		// Compare each entry in the buffer multi-sz list with our szHardwareID.
//		//
//		for (p = buffer; *p && (p < &buffer[buffersize]); p += lstrlen(p) + sizeof(TCHAR))
//		{
//			if (!_tcscmp(szHardwareID, p))
//			{
//				//DT(TEXT("Found! [%s]\n"),p);
//				DebugMesBox(szHardwareID);
//				Found = TRUE;
//				break;
//			}
//		}
//
//		delete[]buffer;
//		buffer = NULL;
//
//		if (Found)
//		{
//			break;
//		}
//	}
//
//	if (GetLastError() != NO_ERROR)
//	{
//		GetErrAndLog(_T("EnumDeviceInfo"));
//	}
//
//cleanup_DeviceInfo:
//
//	err = GetLastError();
//	SetupDiDestroyDeviceInfoList(DeviceInfoSet); // 清空枚举后的设备信息列表
//	SetLastError(err);
//
//	return (err == NO_ERROR) && Found;
//}