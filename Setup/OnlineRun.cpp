#include "stdafx.h"
#include "OnlineRun.h"
#include "GetSysVersion.h"
#include "InfFileHelper.h"
#include "DebugStuff.h"
#include <cfgmgr32.h>

#include "DriverSetupHelper.h"
#include "UDefault.h"

OnlineRun::OnlineRun()
{
	m_hDevInfo = NULL;
	m_nDeviceIndex = -1;
}

OnlineRun::~OnlineRun()
{
}

// 获取inf路径
CString OnlineRun::GetInfPath(LPCTSTR PointMark, BOOL OsX64, LPCTSTR infName)
{
	CString strReturn = _T("");

	//修改使用GetModuleFileName获取当前路径，防止路径改变时，出现获取不正常情况
	CString strAppName;
	GetModuleFileName(NULL, strAppName.GetBuffer(_MAX_PATH), _MAX_PATH);
	//在其后面加上ini文件名
	strAppName.ReleaseBuffer();
	int nPos = strAppName.ReverseFind('\\');
	strAppName = strAppName.Left(nPos + 1);

	TCHAR* byCurPath = new TCHAR[_MAX_PATH];
	ZeroMemory(byCurPath, _MAX_PATH);
	lstrcpy(byCurPath, strAppName);

	DebugMesBox(byCurPath);

	// 拷贝inf文件到系统路径下
	TCHAR OEMSourceMediaLocation[BUFSIZE] = { 0 }; //存放inf根目录路径
	lstrcpy(OEMSourceMediaLocation, byCurPath);

	TCHAR InfPath[BUFSIZE] = { 0 };  //存放inf完整路径

	lstrcat(OEMSourceMediaLocation, PointMark);
	lstrcat(OEMSourceMediaLocation, _T("\\"));
	lstrcpy(InfPath, OEMSourceMediaLocation);
	lstrcat(InfPath, infName);
	strReturn = InfPath;

	return strReturn;
}

// SetupDiGetINFClass 函数指针类型
typedef BOOL(WINAPI* PFN_SetupDiGetINFClassW)(
	PCWSTR InfName,
	GUID* ClassGuid,
	PWSTR ClassName,
	DWORD ClassNameSize,
	PDWORD RequiredSize
	);

// SetupDiGetClassDevs 函数指针类型
typedef HDEVINFO(WINAPI* PFN_SetupDiGetClassDevsW)(
	const GUID* ClassGuid,
	PCWSTR Enumerator,
	HWND hwndParent,
	DWORD Flags
	);

// SetupDiGetDeviceInstallParams 函数指针类型
typedef BOOL(WINAPI* PFN_SetupDiGetDeviceInstallParamsW)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	PSP_DEVINSTALL_PARAMS_W DeviceInstallParams
	);

// SetupDiSetDeviceInstallParams 函数指针类型
typedef BOOL(WINAPI* PFN_SetupDiSetDeviceInstallParamsW)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	PSP_DEVINSTALL_PARAMS_W DeviceInstallParams
	);

// SetupDiBuildDriverInfoList 函数指针类型
typedef BOOL(WINAPI* PFN_SetupDiBuildDriverInfoList)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	DWORD DriverType
	);

// SetupDiEnumDriverInfo 函数指针类型
typedef BOOL(WINAPI* PFN_SetupDiEnumDriverInfoW)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	DWORD DriverType,
	DWORD MemberIndex,
	PSP_DRVINFO_DATA_W DriverInfoData
	);

// SetupDiGetDriverInfoDetail 函数指针类型（修正后的）
typedef BOOL(WINAPI* PFN_SetupDiGetDriverInfoDetailW)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	PSP_DRVINFO_DATA_W DriverInfoData,
	PSP_DRVINFO_DETAIL_DATA_W DriverInfoDetailData,
	DWORD DriverInfoDetailDataSize,
	PDWORD RequiredSize
	);

// SetupDiDestroyDriverInfoList 函数指针类型
typedef BOOL(WINAPI* PFN_SetupDiDestroyDriverInfoList)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	DWORD DriverType
	);

// SetupDiEnumDeviceInfo 函数指针类型
typedef BOOL(WINAPI* PFN_SetupDiEnumDeviceInfo)(
	HDEVINFO DeviceInfoSet,
	DWORD MemberIndex,
	PSP_DEVINFO_DATA DeviceInfoData
	);

// SetupDiGetDeviceRegistryProperty 函数指针类型
typedef BOOL(WINAPI* PFN_SetupDiGetDeviceRegistryPropertyW)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	DWORD Property,
	PDWORD PropertyRegDataType,
	PBYTE PropertyBuffer,
	DWORD PropertyBufferSize,
	PDWORD RequiredSize
	);

// SetupDiGetClassImageIndex 函数指针类型
typedef BOOL(WINAPI* PFN_SetupDiGetClassImageIndex)(
	const SP_CLASSIMAGELIST_DATA* ClassImageListData,
	const GUID* ClassGuid,
	int* ImageIndex
	);

// SetupDiDestroyDeviceInfoList 函数指针类型
typedef BOOL(WINAPI* PFN_SetupDiDestroyDeviceInfoList)(
	HDEVINFO DeviceInfoSet
	);

// 修订后的 OpenInf 函数
BOOL OnlineRun::OpenInf(CString szInfFile)
{
	Reset(); // 重置当前状态

	DebugMesBox(szInfFile); // 调试信息

	CStringArray szArrHwID; // 存储硬件ID数组

	CInfFileHelper infHelper;

	if (!infHelper.OpenInfFile(szInfFile))
	{
		return FALSE; // 打开INF文件失败
	}

	CStringArray szArr;

	infHelper.GetManufacturer(szArr); // 获取硬件制造商

	for (int i = 0; i < szArr.GetSize(); i++)
	{
		CStringArray szArrHwIDTemp; // 临时存储硬件ID
		infHelper.GetHardwareID(szArr[i], szArrHwIDTemp);
		szArrHwID.Append(szArrHwIDTemp); // 合并硬件ID
	}

	m_szInfFile = szInfFile; // 保存INF文件路径

	// 动态加载 SetupAPI.dll
	HMODULE hSetupAPI = LoadLibrary(TEXT("SetupAPI.dll"));
	if (hSetupAPI == NULL)
	{
		GetErrAndLog(_T("无法加载 SetupAPI.dll"));
		return FALSE;
	}

	// 获取所有需要的函数地址，不带后缀
	PFN_SetupDiGetINFClassW pSetupDiGetINFClass =
		(PFN_SetupDiGetINFClassW)GetProcAddress(hSetupAPI, "SetupDiGetINFClassW");
	PFN_SetupDiGetClassDevsW pSetupDiGetClassDevs =
		(PFN_SetupDiGetClassDevsW)GetProcAddress(hSetupAPI, "SetupDiGetClassDevsW");
	PFN_SetupDiGetDeviceInstallParamsW pSetupDiGetDeviceInstallParams =
		(PFN_SetupDiGetDeviceInstallParamsW)GetProcAddress(hSetupAPI, "SetupDiGetDeviceInstallParamsW");
	PFN_SetupDiSetDeviceInstallParamsW pSetupDiSetDeviceInstallParams =
		(PFN_SetupDiSetDeviceInstallParamsW)GetProcAddress(hSetupAPI, "SetupDiSetDeviceInstallParamsW");
	PFN_SetupDiBuildDriverInfoList pSetupDiBuildDriverInfoList =
		(PFN_SetupDiBuildDriverInfoList)GetProcAddress(hSetupAPI, "SetupDiBuildDriverInfoList");
	PFN_SetupDiEnumDriverInfoW pSetupDiEnumDriverInfo =
		(PFN_SetupDiEnumDriverInfoW)GetProcAddress(hSetupAPI, "SetupDiEnumDriverInfoW");
	PFN_SetupDiGetDriverInfoDetailW pSetupDiGetDriverInfoDetail =
		(PFN_SetupDiGetDriverInfoDetailW)GetProcAddress(hSetupAPI, "SetupDiGetDriverInfoDetailW");
	PFN_SetupDiDestroyDriverInfoList pSetupDiDestroyDriverInfoList =
		(PFN_SetupDiDestroyDriverInfoList)GetProcAddress(hSetupAPI, "SetupDiDestroyDriverInfoList");
	PFN_SetupDiEnumDeviceInfo pSetupDiEnumDeviceInfo =
		(PFN_SetupDiEnumDeviceInfo)GetProcAddress(hSetupAPI, "SetupDiEnumDeviceInfo");
	PFN_SetupDiGetDeviceRegistryPropertyW pSetupDiGetDeviceRegistryProperty =
		(PFN_SetupDiGetDeviceRegistryPropertyW)GetProcAddress(hSetupAPI, "SetupDiGetDeviceRegistryPropertyW");
	PFN_SetupDiGetClassImageIndex pSetupDiGetClassImageIndex =
		(PFN_SetupDiGetClassImageIndex)GetProcAddress(hSetupAPI, "SetupDiGetClassImageIndex");
	PFN_SetupDiDestroyDeviceInfoList pSetupDiDestroyDeviceInfoListPtr =
		(PFN_SetupDiDestroyDeviceInfoList)GetProcAddress(hSetupAPI, "SetupDiDestroyDeviceInfoList");

	// 检查所有函数是否成功获取
	if (!pSetupDiGetINFClass || !pSetupDiGetClassDevs || !pSetupDiGetDeviceInstallParams ||
		!pSetupDiSetDeviceInstallParams || !pSetupDiBuildDriverInfoList || !pSetupDiEnumDriverInfo ||
		!pSetupDiGetDriverInfoDetail || !pSetupDiDestroyDriverInfoList ||
		!pSetupDiEnumDeviceInfo || !pSetupDiGetDeviceRegistryProperty || !pSetupDiGetClassImageIndex ||
		!pSetupDiDestroyDeviceInfoListPtr)
	{
		GetErrAndLog(_T("无法获取 SetupAPI 所需的函数地址"));
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	BOOL bOK = FALSE;
	GUID guid;
	TCHAR szClassName[MAX_PATH] = { 0 };

	// 获取INF文件的类GUID和类名
	bOK = pSetupDiGetINFClass(szInfFile, &guid, szClassName, MAX_PATH, NULL);
	if (!bOK)
	{
		DumpErrorMes(_T("SetupDiGetINFClass"));
		pSetupDiDestroyDeviceInfoListPtr(m_hDevInfo); // 使用动态函数销毁
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	// 获取设备信息集合
	m_hDevInfo = pSetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_PROFILE);
	if (m_hDevInfo == INVALID_HANDLE_VALUE)
	{
		DumpErrorMes(_T("SetupDiGetClassDevs"));
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	// 获取设备安装参数
	SP_DEVINSTALL_PARAMS_W sppd = { 0 };
	sppd.cbSize = sizeof(sppd);
	bOK = pSetupDiGetDeviceInstallParams(m_hDevInfo, NULL, &sppd);
	if (!bOK)
	{
		DumpErrorMes(_T("SetupDiGetDeviceInstallParams"));
		pSetupDiDestroyDeviceInfoListPtr(m_hDevInfo);
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	// 设置设备安装参数
	sppd.Flags = DI_ENUMSINGLEINF; // 只枚举单个INF
	sppd.FlagsEx = DI_FLAGSEX_ALLOWEXCLUDEDDRVS; // 允许排除驱动程序
	wcscpy_s(sppd.DriverPath, szInfFile);
	bOK = pSetupDiSetDeviceInstallParams(m_hDevInfo, NULL, &sppd);
	if (!bOK)
	{
		DumpErrorMes(_T("SetupDiSetDeviceInstallParams"));
		pSetupDiDestroyDeviceInfoListPtr(m_hDevInfo);
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	// 构建驱动程序信息列表
	bOK = pSetupDiBuildDriverInfoList(m_hDevInfo, NULL, SPDIT_CLASSDRIVER);
	if (!bOK)
	{
		DumpErrorMes(_T("SetupDiBuildDriverInfoList"));
		pSetupDiDestroyDeviceInfoListPtr(m_hDevInfo);
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	// 枚举驱动程序信息
	int nIndex = 0;
	SP_DRVINFO_DATA_W driverInfo = { 0 };
	driverInfo.cbSize = sizeof(driverInfo);
	TCHAR szHardID[MAX_PATH + 1] = { 0 };

	do
	{
		bOK = pSetupDiEnumDriverInfo(m_hDevInfo, NULL, SPDIT_CLASSDRIVER, nIndex++, &driverInfo);
		if (bOK)
		{
			// 分配足够的缓冲区
			DWORD dwRequiredSize = 0;
			pSetupDiGetDriverInfoDetail(m_hDevInfo, NULL, &driverInfo, NULL, 0, &dwRequiredSize);

			PSP_DRVINFO_DETAIL_DATA_W pDriverDetail = (PSP_DRVINFO_DETAIL_DATA_W)new BYTE[dwRequiredSize];
			pDriverDetail->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA_W);

			bOK = pSetupDiGetDriverInfoDetail(
				m_hDevInfo,
				NULL,               // DeviceInfoData，可以为 NULL
				&driverInfo,        // DriverInfoData
				pDriverDetail,      // DriverInfoDetailData
				dwRequiredSize,     // DriverInfoDetailDataSize
				NULL                // RequiredSize
			);
			if (bOK)
			{
				wcsncpy_s(szHardID, pDriverDetail->HardwareID, MAX_PATH);
			}

			delete[] pDriverDetail; // 释放分配的内存
		}
	} while (bOK);

	// 销毁驱动程序信息列表
	pSetupDiDestroyDriverInfoList(m_hDevInfo, NULL, SPDIT_CLASSDRIVER);

	// 枚举设备信息
	nIndex = -1;
	do
	{
		SP_DEVINFO_DATA spdd = { 0 };
		spdd.cbSize = sizeof(spdd);
		bOK = pSetupDiEnumDeviceInfo(m_hDevInfo, ++nIndex, &spdd);
		if (bOK)
		{
			TCHAR szBuf[2000] = { 0 };
			// 获取硬件ID属性
			pSetupDiGetDeviceRegistryProperty(m_hDevInfo, &spdd, SPDRP_HARDWAREID, NULL, (PBYTE)szBuf, sizeof(szBuf), NULL);

			// 判断硬件ID是否匹配
			if (CInfFileHelper::MatchHwID(szArrHwID, szBuf))
			{
				m_szCurHwID = szBuf; // 保存当前硬件ID

				// 获取设备描述
				pSetupDiGetDeviceRegistryProperty(m_hDevInfo, &spdd, SPDRP_DEVICEDESC, NULL, (PBYTE)szBuf, sizeof(szBuf), NULL);

				int nImageIndex = 0;
				m_nDeviceIndex = nIndex;

				// 获取类图像索引
				pSetupDiGetClassImageIndex(&m_sid, &spdd.ClassGuid, &nImageIndex);
			}
		}
	} while (bOK);

	// 检查是否找到匹配的硬件ID
	if (m_szCurHwID.IsEmpty())
	{
		GetErrAndLog(_T("未找到相符合的设备ID"));
		pSetupDiDestroyDeviceInfoListPtr(m_hDevInfo);
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	// 释放 SetupAPI.dll
	pSetupDiDestroyDeviceInfoListPtr(m_hDevInfo);
	FreeLibrary(hSetupAPI);

	return TRUE; // 成功
}

// 修订后的 Reset 函数
//void OnlineRun::Reset()
//{
//	if (m_hDevInfo)
//	{
//		// 动态加载 SetupAPI.dll
//		HMODULE hSetupAPI = LoadLibrary(TEXT("SetupAPI.dll"));
//		if (hSetupAPI)
//		{
//			// 定义函数指针类型
//			typedef BOOL(WINAPI* PFN_SetupDiDestroyDeviceInfoList)(
//				HDEVINFO DeviceInfoSet
//				);
//			// 获取函数地址
//			PFN_SetupDiDestroyDeviceInfoList pSetupDiDestroyDeviceInfoList =
//				(PFN_SetupDiDestroyDeviceInfoList)GetProcAddress(hSetupAPI, "SetupDiDestroyDeviceInfoList");
//
//			if (pSetupDiDestroyDeviceInfoList)
//			{
//				// 调用函数销毁设备信息列表
//				pSetupDiDestroyDeviceInfoList(m_hDevInfo);
//			}
//			else
//			{
//				// 获取函数地址失败，处理错误
//				GetErrAndLog(_T("无法获取函数 SetupDiDestroyDeviceInfoList 的地址"));
//			}
//			// 释放动态库
//			FreeLibrary(hSetupAPI);
//		}
//		else
//		{
//			// 加载库失败，处理错误
//			GetErrAndLog(_T("无法加载 SetupAPI.dll"));
//		}
//		m_hDevInfo = NULL;
//	}
//	m_szCurHwID.Empty();
//	m_szInfFile.Empty();
//}

void OnlineRun::Reset()
{
	if (m_hDevInfo)
	{
		// 动态加载 SetupAPI.dll
		HMODULE hSetupAPI = LoadLibrary(TEXT("SetupAPI.dll"));
		if (hSetupAPI)
		{
			// 定义函数指针类型
			typedef BOOL(WINAPI* PFN_SetupDiDestroyDeviceInfoList)(
				HDEVINFO DeviceInfoSet
				);
			// 获取函数地址
			PFN_SetupDiDestroyDeviceInfoList pSetupDiDestroyDeviceInfoList =
				(PFN_SetupDiDestroyDeviceInfoList)GetProcAddress(hSetupAPI, "SetupDiDestroyDeviceInfoList");

			if (pSetupDiDestroyDeviceInfoList)
			{
				// 调用函数销毁设备信息列表
				pSetupDiDestroyDeviceInfoList(m_hDevInfo);
			}
			else
			{
				// 获取函数地址失败，处理错误
				GetErrAndLog(_T("无法获取函数 SetupDiDestroyDeviceInfoList 的地址"));
			}
			// 释放动态库
			FreeLibrary(hSetupAPI);
		}
		else
		{
			// 加载库失败，处理错误
			GetErrAndLog(_T("无法加载 SetupAPI.dll"));
		}
		m_hDevInfo = NULL;
	}
	m_szCurHwID.Empty();
	m_szInfFile.Empty();

	// 添加扫描检测硬件改动的代码
	// 动态加载 cfgmgr32.dll
	HMODULE hCfgMgr32 = LoadLibrary(TEXT("cfgmgr32.dll"));
	if (hCfgMgr32)
	{
		// 定义函数指针类型
		typedef CONFIGRET(WINAPI* PFN_CM_Locate_DevNodeW)(
			PDEVINST pdnDevInst,
			DEVINSTID_W pDeviceID,
			ULONG ulFlags
			);
		typedef CONFIGRET(WINAPI* PFN_CM_Reenumerate_DevNode)(
			DEVINST dnDevInst,
			ULONG ulFlags
			);

		// 获取函数地址
		PFN_CM_Locate_DevNodeW pCM_Locate_DevNodeW =
			(PFN_CM_Locate_DevNodeW)GetProcAddress(hCfgMgr32, "CM_Locate_DevNodeW");
		PFN_CM_Reenumerate_DevNode pCM_Reenumerate_DevNode =
			(PFN_CM_Reenumerate_DevNode)GetProcAddress(hCfgMgr32, "CM_Reenumerate_DevNode");

		if (pCM_Locate_DevNodeW && pCM_Reenumerate_DevNode)
		{
			DEVINST devRoot = 0;
			CONFIGRET cr = pCM_Locate_DevNodeW(&devRoot, NULL, CM_LOCATE_DEVNODE_NORMAL);
			if (cr == CR_SUCCESS)
			{
				// 触发硬件重新枚举
				cr = pCM_Reenumerate_DevNode(devRoot, CM_REENUMERATE_SYNCHRONOUS);
				if (cr != CR_SUCCESS)
				{
					// 处理错误
					CString strError;
					strError.Format(_T("CM_Reenumerate_DevNode 执行失败，错误码：%lu"), cr);
					GetErrAndLog(strError);
				}
			}
			else
			{
				// 处理错误
				CString strError;
				strError.Format(_T("CM_Locate_DevNodeW 执行失败，错误码：%lu"), cr);
				GetErrAndLog(strError);
			}
		}
		else
		{
			// 获取函数地址失败，处理错误
			GetErrAndLog(_T("无法获取函数 CM_Locate_DevNodeW 或 CM_Reenumerate_DevNode 的地址"));
		}

		// 释放动态库
		FreeLibrary(hCfgMgr32);
	}
	else
	{
		// 加载库失败，处理错误
		GetErrAndLog(_T("无法加载 cfgmgr32.dll"));
	}
}

//// 打开inf文件
//BOOL OnlineRun::OpenInf(CString szInfFile)
//{
//	Reset();
//	DebugMesBox(szInfFile);
//	CStringArray szArrHwID;
//
//	CInfFileHelper infHelper;
//	if (!infHelper.OpenInfFile(szInfFile))
//	{
//		return FALSE;
//	}
//
//	CStringArray szArr;
//	infHelper.GetManufacturer(szArr);					//Get Hardware Manufacture
//	for (int i = 0; i < szArr.GetSize(); i++)
//	{
//		CStringArray szArrHwIDTemp;						 //Get HdID
//		infHelper.GetHardwareID(szArr[i], szArrHwIDTemp);
//		szArrHwID.Append(szArrHwIDTemp);
//	}
//
//	m_szInfFile = szInfFile;
//
//	BOOL bOK = FALSE;
//	GUID guid;
//	TCHAR szClassName[MAX_PATH];
//	bOK = SetupDiGetINFClass(szInfFile, &guid, szClassName, MAX_PATH, 0);    //获取端口名称和GUID
//	if (!bOK)
//	{
//		DumpErrorMes(_T("SetupDiGetINFClass"));
//		return FALSE;
//	}
//
//	m_hDevInfo = SetupDiGetClassDevs(&guid, NULL, NULL,
//		DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_PROFILE);  //根据指定的USB设备的GUID获取设备信息集
//
//	SP_DEVINSTALL_PARAMS sppd = { 0 };
//	sppd.cbSize = sizeof(sppd);
//	bOK = SetupDiGetDeviceInstallParams(m_hDevInfo, NULL, &sppd);  //检索设备安装参数设置设备的信息或特定设备的信息元素
//
//	sppd.Flags = DI_ENUMSINGLEINF;  //标志，以表明指定SP_DEVINSTALL_PARAMS.DriverPath只有在INF应搜索。
//
//	sppd.FlagsEx = DI_FLAGSEX_ALLOWEXCLUDEDDRVS;
//	wcscpy_s(sppd.DriverPath, szInfFile);
//	bOK = SetupDiSetDeviceInstallParams(m_hDevInfo, NULL, &sppd);
//
//	bOK = SetupDiBuildDriverInfoList(m_hDevInfo, NULL, SPDIT_CLASSDRIVER); //建立一个与一个特定的设备，或与全球级的设备驱动程序列表中的信息相关的驱动程序的列表。
//
//	int nIndex = 0;
//	SP_DRVINFO_DATA driverInfo = { 0 };
//	driverInfo.cbSize = sizeof(driverInfo);
//	TCHAR szHardID[MAX_PATH + 1];
//	szHardID[0] = '\0';
//	do
//	{
//		bOK = SetupDiEnumDriverInfo(m_hDevInfo, 0, SPDIT_CLASSDRIVER, nIndex++, &driverInfo);//列举了驱动程序列表的成员
//		if (bOK)
//		{
//			char szBuf[2000];
//			ZeroMemory(szBuf, 2000);
//			DWORD dwRequiredSize = 0;
//			PSP_DRVINFO_DETAIL_DATA pDriverDetail = (PSP_DRVINFO_DETAIL_DATA)szBuf;
//			pDriverDetail->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
//			if (SetupDiGetDriverInfoDetail(m_hDevInfo, NULL, &driverInfo,  //检索驱动程序信息细节设置设备的信息或在设备信息集的特定设备的信息元素。
//				pDriverDetail, 2000, &dwRequiredSize))
//			{
//				wcsncpy_s(szHardID, pDriverDetail->HardwareID, MAX_PATH);
//			}
//		}
//	} while (bOK);
//	SetupDiDestroyDriverInfoList(m_hDevInfo, NULL, SPDIT_CLASSDRIVER);  //删除驱动程序列表
//	nIndex = -1;
//
//	do
//	{
//		SP_DEVINFO_DATA spdd = { 0 };
//		spdd.cbSize = sizeof(spdd);
//		bOK = SetupDiEnumDeviceInfo(m_hDevInfo, ++nIndex, &spdd);   //枚举指定设备信息集合的成员
//		if (bOK)
//		{
//			TCHAR szBuf[2000];
//			SetupDiGetDeviceRegistryProperty(m_hDevInfo, &spdd, SPDRP_HARDWAREID, NULL, (PBYTE)szBuf, 2000, NULL);
//
//			if (CInfFileHelper::MatchHwID(szArrHwID, szBuf))
//			{
//				m_szCurHwID = szBuf;
//
//				SetupDiGetDeviceRegistryProperty(m_hDevInfo, &spdd, SPDRP_DEVICEDESC,
//					NULL, (PBYTE)szBuf, 2000, NULL);
//
//				int nImageIndex = 0;
//				m_nDeviceIndex = nIndex;
//				SetupDiGetClassImageIndex(&m_sid, &spdd.ClassGuid, &nImageIndex);
//			}
//		}
//	} while (bOK);
//
//	if (m_szCurHwID == _T(""))
//	{
//		GetErrAndLog(_T("未找到相符合的设备ID"));
//		return FALSE;
//	}
//
//	return TRUE;
//}

// void OnlineRun::Reset()
// {
// 	if (m_hDevInfo)
// 	{
// 		SetupDiDestroyDeviceInfoList(m_hDevInfo);// 清空枚举后的设备信息列表
// 		m_hDevInfo = NULL;
// 	}
//
// 	m_szCurHwID.Empty();
// 	m_szInfFile.Empty();
// }
//

BOOL OnlineRun::AddNewDevice()
{
	BOOL bRetrun = FALSE;

	CDriverSetupHelper ds;
	if (ds.Install(m_szInfFile, m_szCurHwID))
	{
		DebugMesBox(_T("安装成功"));
		bRetrun = TRUE;
	}

	return bRetrun;
}