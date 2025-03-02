#include "stdafx.h"
#include "ExportHead.h"
#include <Winspool.h>
#include "UDefault.h"
#include "AutoRun.h"
#include "OnlineRun.h"
//#include "UninstallDriver.h"
#include "DebugStuff.h"
#include "IOControl.h"
#include "StrTranslate.h"
#include "GetSysVersion.h"
#include "LPTCtrl.h"
#include "ntddpar.h"
//#include "DebugStuff.h"
#include <Winsvc.h>
#include <cfgmgr32.h>

//错误代码列表
#define SUCCESS 0									// 成功
#define ERROR_COPYFILE  1					//	拷贝文件失败
#define ERROR_SETUPINF 2					//	安装inf文件失败
#define ERROR_ADDPIRNTDRIVER 3	//	增加打印机驱动失败
#define ERROR_ADDPIRNT 4					//	增加打印机失败
#define ERROR_ADDREGEDIT 5			//	写入注册表失败
#define ERROR_SETDEFAUTL 6				//	设置默认打印机失败
#define ERROR_GETSYSPATH 7			//	获取系统路径失败

//使用预安装模式安装打印机驱动
/***************
参数：
DlghWnd ： 对话框句柄
Seriver ： 系列
Model   ： 型号
InstallName ：安装名称
Port ：端口号
PointMark  ：针数标识 【24针：24Pin，12针：12Pin，9针：9Pin，热敏：Thermal，热转印：ThermalTran】
OsX64 ：是否为64位系统
infName ：安装的驱动的Inf名称

错误代码见上文列表

***************/
int   PreInstalled(HWND DlghWnd, LPCTSTR Seriver, LPCTSTR Model, LPCTSTR InstallName, LPCTSTR Port, LPCTSTR GPD, LPCTSTR PointMark, BOOL OsX64, InstallFile File, LPCTSTR infName)
{
	int iRet = SUCCESS;

	//打印机句柄
	HANDLE hPrinter;
	//存放打印机名字和端口
	CString   lpDeviceName = InstallName;

	//枚举检查是否已安装驱动，如果已安装则重命名为副本n
	int iCpyCounts = 0;
	CString strCpy;

	//打开指定的打印机，并获取打印机的句柄
	while (OpenPrinter(lpDeviceName.GetBuffer(), &hPrinter, NULL))
	{
		ClosePrinter(hPrinter);

		strCpy = IDS_CopyName;
		iCpyCounts++;
		strCpy.Format(strCpy, iCpyCounts);

		lpDeviceName = InstallName;
		lpDeviceName += strCpy;

		lpDeviceName.ReleaseBuffer();
	}
	DebugMesBox(lpDeviceName);

	iRet = CopyFilesToSys(DlghWnd, Seriver, Model, GPD, PointMark, OsX64, File, infName);
	//调用函数复制文件至指定路径
	if (!iRet)
	{
		//成功
		iRet = AddPrinterToSys(lpDeviceName.GetBuffer(), Port, GPD, File, OsX64);
		lpDeviceName.ReleaseBuffer();

		//调用函数增加打印机至计算机
		if (!iRet)
		{
			// 设置默认打印机
			if (SetPrinterDefault(lpDeviceName.GetBuffer()))
			{
				iRet = SUCCESS;								//	成功
			}
			else
			{
				iRet = ERROR_SETDEFAUTL;			//	设为默认打印机失败
			}
			lpDeviceName.ReleaseBuffer();
		}
		else
		{
			switch (iRet)
			{
			case 1:
				iRet = ERROR_GETSYSPATH;				//	获取系统路径失败
				break;
			case 2:
				iRet = ERROR_ADDPIRNTDRIVER;		//	增加打印机驱动失败
				break;
			case 3:
				iRet = ERROR_ADDPIRNT;					//	增加打印机失败
				break;
			default:
				break;
			}
		}
	}
	else
	{
		switch (iRet)
		{
		case 1:
			iRet = ERROR_GETSYSPATH;		//	获取系统路径失败
			break;
		case 2:
			iRet = ERROR_COPYFILE;			//	拷贝文件失败
			break;
		case 3:
			iRet = ERROR_SETUPINF;			//	安装inf文件失败
			break;
		default:
			break;
		}
	}
	return iRet;
}

//使用在线模式安装打印机驱动
/***************
参数：
DlghWnd ： 对话框句柄
Model：打印机型号
InstallName ： 安装的驱动名称
PointMark  ：针数标识 【24针：24Pin，12针：12Pin，9针：9Pin，热敏：Thermal，热转印：ThermalTran】
GPD ： GPD文件名称
OsX64 ： 是否为X64系统
MFG ：产家信息，例如START 用来生成pnpid
bNewModel ：是否为新增机型
infName ：安装的驱动的Inf名称
返回值：
新的inf的路径：
***************/
BOOL  OnlineInstalled(HWND DlghWnd, LPCTSTR Model, LPCTSTR InstallName, LPCTSTR PointMark, LPCTSTR GPD, BOOL OsX64, LPCTSTR  MFG, BOOL bNewModel, LPCTSTR infName)
{
	BOOL bRet = FALSE;

	//更新驱动设备
	OnlineRun setup;
	CString infPath = setup.GetInfPath(PointMark, OsX64, infName);

	if (setup.OpenInf(infPath))
	{
		if (setup.AddNewDevice())
		{
			SetPrinterDefault(InstallName);
			bRet = TRUE;
		}
	}

	return bRet;
}

//获取打印机驱动信息
/***************
参数：
Model ： 驱动型号
返回值：
my_die :自定义结构体
***************/
MY_DRIVER_INFO* GetPrintDriverInfo(LPCTSTR Model)
{
	MY_DRIVER_INFO* my_die = new  MY_DRIVER_INFO;
	DRIVER_INFO_6* die6 = new DRIVER_INFO_6;
	DWORD dSize = 0;
	//枚举所有打印机驱动
	int iNum = EnumAllPrintDriver(die6, dSize);
	if (iNum <= 0)
	{
		delete my_die;
		my_die = NULL;

		return NULL;
	}
	else
	{
		DRIVER_INFO_6* diePos = die6;

		for (int i = 0; i < iNum; i++)
		{
			if (diePos->pName == Model)
			{
				my_die->strEnvironment = diePos->pEnvironment;
				my_die->strName = diePos->pName;
				my_die->dwVersion = diePos->cVersion;
				break;
			}
			diePos++;
		}
	}
	delete die6;
	die6 = NULL;

	return my_die;
}

// //卸载打印机驱动
// /***************
// 参数：
// my_die :自定义结构体
// InstallName ： 安装的驱动名称
// 返回值：
// 卸载成功：TRUE
// 卸载失败：FALSE
// ***************/
// BOOL  UninstallDriver(LPCTSTR InstallName, MY_DRIVER_INFO *die)
// {
// 	BOOL bRet = TRUE;
//
// 	JOB_INFO_2 * Job = NULL;
// 	//有此型号了
// 	//记录原有几个打印作业
// 	int iPages = 0;
// 	//有打印作业
// 	if (!DeletePrintPages(InstallName, iPages))
// 	{
// 		//MesError(_T("9,您已经安装了驱动 ") + PrintModel + _T(" ，且还有打印任务尚未完成，请先取消打印任务后再进行驱动程序安装。"));
// 		//删除打印任务失败
// 		GetErrAndLog(_T("删除打印作业失败"));
// 		return FALSE;
// 	}
//
// 	//只有原来有打印作业时候，才重启
// 	if (iPages)
// 	{
// 		//重启explorer
// 		//RetBootExpor();
// 		Sleep(500);
// 		//枚举是否有打印作业
// 		while (GetPrintPages(InstallName, Job) > 0)
// 		{
// 			Sleep(2000);
// 		}
// 		Sleep(1000);
// 		delete Job;
// 		Job = NULL;
// 	}
//
// 	////关闭Print Spooler服务
// 	//if (!CloseSampleService(_T("Spooler")))
// 	//{
// 	//	GetErrAndLog(_T("关闭服务失败"));
// 	//	return FALSE;
// 	//}
// 	//Sleep(2000);
// 	////删除打印错误任务残留文件
// 	//if (!DeletePrintErrorFile())
// 	//{
// 	//	return FALSE;
// 	//}
// 	////删除Inf
// 	//if (!DeleteInf(InstallName))
// 	//{
// 	//	return FALSE;
// 	//}
// 	////删除备份
// 	//if (!DeleteBackUp(InstallName))
// 	//{
// 	//	return FALSE;
// 	//}
// 	////开启Print Spooler服务
// 	//if (!StartSampleService(_T("Spooler")))
// 	//{
// 	//	GetErrAndLog(_T("开启服务失败"));
// 	//	return FALSE;
// 	//}
// 	//Sleep(2000);
// 	////删除打印机及打印机驱动
// 	//if (!DeletePrint(InstallName))
// 	//{
// 	//	return FALSE;
// 	//}
// 	////重启服务
// 	//if (!RetBootService(_T("Spooler")))
// 	//{
// 	//	return FALSE;
// 	//}
// 	//Sleep(2000);
// 	//if (!DeletePrintDriver1(die))
// 	//{
// 	//	return FALSE;
// 	//}
// 	//ScanForHardwareChange();
// 	//return bRet;
//
// 	//关闭Print Spooler服务
// 	if (!CloseSampleService(_T("Spooler")))
// 	{
// 		GetErrAndLog(_T("关闭服务失败"));
// 		return FALSE;
// 	}
//
// 	Sleep(2000);
//
// 	//删除打印错误任务残留文件
// 	if (!DeletePrintErrorFile())
// 	{
// 		return FALSE;
// 	}
//
// 	//删除Inf
// 	if (!DeleteInf(InstallName))
// 	{
// 		return FALSE;
// 	}
// 	//删除备份
// 	if (!DeleteBackUp(InstallName))
// 	{
// 		return FALSE;
// 	}
//
// 	//开启Print Spooler服务
// 	if (!StartSampleService(_T("Spooler")))
// 	{
// 		GetErrAndLog(_T("开启服务失败"));
// 		return FALSE;
// 	}
//
// 	Sleep(2000);
//
// 	//删除打印机及打印机驱动
// 	if (!DeletePrint(InstallName))
// 	{
// 		return FALSE;
// 	}
//
// 	//重启服务
// 	if (!RetBootService(_T("Spooler")))
// 	{
// 		return FALSE;
// 	}
//
// 	Sleep(2000);
//
// 	if (!DeletePrintDriver1(die))
// 	{
// 		return FALSE;
// 	}
//
// 	//卸载设备管理器设备
// 	if (!UninstallWDMDriver(InstallName))
// 	{
// 		return FALSE;
// 	}
//
// 	//删除Inf
// 	if (!DeleteInf(InstallName))
// 	{
// 		return FALSE;
// 	}
// 	if (!DeleteBackUp(InstallName))
// 	{
// 		return FALSE;
// 	}
// 	//开启Print Spooler服务
// 	if (!StartSampleService(_T("Spooler")))
// 	{
// 		GetErrAndLog(_T("开启服务失败"));
// 		return FALSE;
// 	}
//
// 	ScanForHardwareChange();
//
// 	Sleep(1000);
// 	return bRet;
// }
//
//
//通过Usb口获取产家信息，设备名称，仿真等
/***************
参数：
PrintName :打印机型号
Command ： 仿真指令
Provider ：产家名称
SelectPro： 筛选条件
返回值：
获取成功：TRUE
获取失败：FALSE
***************/
BOOL  GetUSBPrintId(LPTSTR& PrintName, LPTSTR& Provider, LPTSTR& Command, LPTSTR SelectPro)
{
	BOOL bRet = 0;

	CString	 strModel = _T("");
	CString	 strProduction = _T("");
	CString	 strSimulation = _T("");

	//初始化代码
	HANDLE Port = InitPrinter();

	if (Port == INVALID_HANDLE_VALUE)
	{
		GetErrAndLog(_T("打开端口失败"));
		return FALSE;
	}

	DWORD dwStatus = 0;
	// 异步方式对端口进行写入操作
	OVERLAPPED ovInternal;
	memset(&ovInternal, 0, sizeof(OVERLAPPED));

	// 创建人工重置事件对象，默认为无信号状态
	ovInternal.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ovInternal.hEvent != NULL)			// 创建事件，开始选择异步方式读端口
	{
		LPOVERLAPPED lpOverlapped = &ovInternal;
		DWORD dwBytesReturned = 0;
		UCHAR Packet[512] = { 0 };

		BOOL Success = DeviceIoControl(Port,
			IOCTL_USBPRINT_GET_1284_ID,
			NULL,
			0,
			&Packet,
			sizeof(Packet),
			&dwBytesReturned,
			NULL);

		if (!Success)
		{
			int i = GetLastError();
			//如果是-数据错误 (循环冗余检查)错误，则不算错误
			//DumpErrorMes(_T("DeviceIoControl"));
			GetErrAndLog(_T("DeviceIoControl"));
			bRet = FALSE;
		}
		else
		{
			bRet = TRUE;
			LPTSTR DeviceID;
			DeviceID = MultiToWChar((LPSTR)Packet, dwBytesReturned);
			strModel = ResolveUsbPrintId(DeviceID);
			strProduction = ResolveUsbPrintProvider(DeviceID);

			//筛选厂家信息
			if (SelectPro != _T(""))
			{
				//判断回读的产家信息是否与筛选产家是一致的
				CString str = SelectPro;
				CString str1 = strProduction;
				str1.Trim();
				if (str != str1)
				{
					/*PrintName = _T("");
					Provider = _T("");*/
				}
				else
				{
					strSimulation = ResolveUsbPrintCommand(DeviceID);

					memcpy(PrintName, strModel, strModel.GetLength() * 2);
					memcpy(Provider, strProduction, strProduction.GetLength() * 2);
					memcpy(Command, strSimulation, strSimulation.GetLength() * 2);
				}
			}
			else
			{
				strSimulation = ResolveUsbPrintCommand(DeviceID);
				memcpy(PrintName, strModel, strModel.GetLength() * 2);
				memcpy(Provider, strProduction, strProduction.GetLength() * 2);
				memcpy(Command, strSimulation, strSimulation.GetLength() * 2);
			}
			delete DeviceID;
			DeviceID = NULL;
		}
	}
	else
	{
		GetErrAndLog(_T("CreateEvent"));
		bRet = FALSE;
	}
	CloseUSBPort(Port);
	return bRet;
}

//获取设备ID，CMD，产家信息
BOOL  GetPrintID_LPT(LPTSTR& Model, LPTSTR& Production, LPTSTR& Simulation, LPTSTR SelectPro)
{
	BOOL bRet = FALSE;

	CString	 strModel = _T("");
	CString	 strProduction = _T("");
	CString	 strSimulation = _T("");

	HANDLE hPort = OpenLPTPort(_T("LPT"));

	if (hPort == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	DWORD dwBufferSize = 0;
	UCHAR Packet[512] = { 0 };

	BOOL Success = DeviceIoControl(hPort,
		IOCTL_PAR_QUERY_DEVICE_ID,
		NULL, 0,
		(LPVOID)Packet,
		sizeof(Packet),
		&dwBufferSize,
		0);

	if (!Success)
	{
		//如果是-数据错误 (循环冗余检查)错误，则不算错误
		GetErrAndLog(_T("DeviceIoControl"));
		bRet = FALSE;
	}
	else
	{
		bRet = TRUE;
		LPTSTR DeviceID = _T("");
		DeviceID = MultiToWChar((LPSTR)Packet, dwBufferSize);
		strModel = ResolvePrintId(DeviceID);
		strProduction = ResolvePrintProvider(DeviceID);

		//如果产家信息不是实达，不返回机型信息和仿真信息
		if (strProduction == _T("START") || strProduction == _T("START "))
		{
			strSimulation = ResolveUsbPrintCommand(DeviceID);
			try
			{
				bRet = TRUE;
				memcpy(Model, strModel, strModel.GetLength() * 2 + 2);
				memcpy(Production, strProduction, strProduction.GetLength() * 2 + 2);
				memcpy(Simulation, strSimulation, strSimulation.GetLength() * 2 + 2);
			}
			catch (...)
			{
				bRet = FALSE;
			};
		}

		////筛选厂家信息
		//if (SelectPro != _T(""))
		//{
		//	//判断回读的产家信息是否与筛选产家是一致的
		//	CString str = SelectPro;
		//	CString str1 = strProduction;
		//	str1.Trim();
		//	if (str != str1)
		//	{
		//		/*Model = _T("");
		//		Production = _T("");*/
		//	}
		//	else
		//	{
		//		strSimulation = ResolveUsbPrintCommand(DeviceID);
		//		memcpy(Model, strModel, strModel.GetLength() * 2);
		//		memcpy(Production, strProduction, strProduction.GetLength() * 2);
		//		memcpy(Simulation, strSimulation, strSimulation.GetLength() * 2);
		//	}
		//}
		//else
		//{
		//	strSimulation = ResolveUsbPrintCommand(DeviceID);
		//	memcpy(Model, strModel, strModel.GetLength() * 2);
		//	memcpy(Production, strProduction, strProduction.GetLength() * 2);
		//	memcpy(Simulation, strSimulation, strSimulation.GetLength() * 2);
		//}

		delete DeviceID;
		DeviceID = NULL;
	}

	return bRet;
}

//查找当前连接的USB端口并返回序号
/***************
参数：
VIDPID :VIDPID 筛选条件  例如：_T("vid_xxxx&pid_xxxx")  x:0-9数字
返回值：
未找到端口返回 0：，否则返回端口号码USB001 = 1
***************/
int  FindUSBPortID(LPCTSTR VIDPID)
{
	int iPortID = 0;

	// 获取当前系统标识
	DWORD dwVersion = GetNtVersion();

	ULONG NumberDeviceInterfaces, NumSelDeviceInfaces;
	HDEVINFO					hardwareDeviceInfo;		// 设备信息集数据结构
	SP_DEVINFO_DATA				deviceInfoData;			// 指定接口的设备信息数据结构
	SP_INTERFACE_DEVICE_DATA	deviceInterfaceData;	// 指定设备接口数据结构

	NumberDeviceInterfaces = 0;						// 所有连接到本机上的设备USB设备数目
	NumSelDeviceInfaces = 0;						// 指定的设备数目

	//GUID ClassGuid = USB_GUID;		 // 指定USB设备GUID
	GUID ClassGuid = GUID_CLASS_I82930_BULK;
	LPGUID  pGuid = &ClassGuid;

	hardwareDeviceInfo = SetupDiGetClassDevs(pGuid,
		NULL, NULL, (DIGCF_PRESENT | DIGCF_INTERFACEDEVICE)); // 根据指定的USB设备的GUID获取设备信息集

	if (hardwareDeviceInfo == INVALID_HANDLE_VALUE)			// 无法获取指定设备的设备集信息，弹出错误提示框
	{
		GetErrAndLog(_T("无法获取指定设备信息集!"));

		SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);

		return	0;
	}
	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);			// 设定缓冲以容纳被访问接口的设备信息
	deviceInterfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);	// 设定足够大的缓冲区以容纳完整的接口信息结构体

	// 循环调用枚举设备接口函数SetupDiEnumDeviceInterfaces查找当前连接在主机上的设备接口，
	// 直到函数返回值false,无法查询到更多接口。
	BOOLEAN bGetDevInface = TRUE;

	CString SelDevicePath;	// 指定要操作的设备端口路径

	do
	{
		bGetDevInface = SetupDiEnumDeviceInterfaces(hardwareDeviceInfo, NULL, pGuid, \
			NumberDeviceInterfaces, &deviceInterfaceData);

		if (bGetDevInface)
		{
			// 如果获得指向设备接口数据结构，则进一步读取设备接口详细信息

			// 第一次调用SetupDiGetInterfaceDeviceDetail,获得用于存放接口详细信息数据结构的缓冲区大小
			PSP_INTERFACE_DEVICE_DETAIL_DATA pDeviceInterfaceDetailData = NULL;
			ULONG predictedLength = 0;
			ULONG requireLength = 0;

			SetupDiGetInterfaceDeviceDetail(hardwareDeviceInfo, &deviceInterfaceData, NULL, \
				0, &requireLength, &deviceInfoData);

			predictedLength = requireLength;

			pDeviceInterfaceDetailData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(predictedLength);
			pDeviceInterfaceDetailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

			// 再次调用SetupDiGetInterfaceDeviceDetail，获得接口详细信息
			if (SetupDiGetInterfaceDeviceDetail(hardwareDeviceInfo, \
				& deviceInterfaceData, pDeviceInterfaceDetailData, \
				predictedLength, &requireLength, &deviceInfoData))
			{
				CString strDevicePath = pDeviceInterfaceDetailData->DevicePath;

				// 判断当前接口是否为我们要连接的USB接口；
				// 如果是指定的USB接口，则打开端口，准备用于读写操作；
				// 如果不是指定的usb接口，则继续搜索直到搜索完全部USB端口。

				CString strMessage = _T("当前设备接口的详细信息: ") + strDevicePath;
				DebugMesBox(strMessage);
				if (strDevicePath.Find(VIDPID) != -1)	// 找到匹配的字符串
				{	// 1
					// 通过注册表查询更多硬件信息
					HKEY hDeviceInfo = NULL;
					CString strHkeyName;
					CString strGuid;
					strGuid = _T("{28d78fad-5a12-11d1-ae5b-0000f803a8c2}");
					strHkeyName = _T("SYSTEM\\CurrentControlSet\\Control\\DeviceClasses\\");
					strHkeyName = strHkeyName + strGuid;

					CString strTemp = strDevicePath;
					if (dwVersion)	// Win2K~
					{
						strTemp.Replace(_T("\\?\\"), _T("##?#"));
					}
					else			// Win98/95
					{
						strTemp.Replace(_T("\\.\\"), _T("##.#"));
					}

					strHkeyName = strHkeyName + strTemp;

					strHkeyName = strHkeyName + _T("\\");
					strHkeyName = strHkeyName + _T("#");
					strHkeyName = strHkeyName + _T("\\");
					strHkeyName = strHkeyName + _T("Device Parameters");

					LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strHkeyName, 0, KEY_READ, &hDeviceInfo);

					if (lResult == ERROR_SUCCESS)
					{ // 2
						CString strUser = _T("Port Number");						//要查询的键名称
						DWORD keyType = REG_DWORD;						//定义数据类型
						DWORD getValue;
						DWORD DataLen = sizeof(DWORD);									//定义数据长度

						long ret1 = RegQueryValueEx(hDeviceInfo, strUser, NULL, &keyType, (LPBYTE)&getValue, &DataLen);
						if (ret1 != ERROR_SUCCESS)
						{ // 3
							GetErrAndLog(_T("错误：无法读取注册表信息"));
							free(pDeviceInterfaceDetailData);
							return 0;
						} //3
						else				// 读取注册表键值成功，校验是否为指定的硬件型号信息
						{	//3
							iPortID = getValue;		// 找到指定的端口号
							//if (getValue == iPortNum)				// 找到指定的端口号
							//{ // 4
							//	NumSelDeviceInfaces++;
							//	SelDevicePath = pDeviceInterfaceDetailData->DevicePath;
							//} // 4 //
						}  // 3 //
					}	// 2 //
					else
					{
						GetErrAndLog(_T("错误：打开指定设备类的注册表项"));
						free(pDeviceInterfaceDetailData);
						return 0;
					}
				}	// 1 //
			}   // 调用调用SetupDiGetInterfaceDeviceDetail获取当前设备接口详细信息

			free(pDeviceInterfaceDetailData);	// 释放存放当前接口详细信息的缓存区

			NumberDeviceInterfaces++;
		}
	} while (bGetDevInface);

	if (ERROR_NO_MORE_ITEMS != GetLastError())
	{
		DebugMesBox(_T("调用SetupDiEnumDeviceInterfaces枚举函数错误，\
					   						 无法获得设备接口详细信息!"));
	}

	SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);		// 清空枚举后的设备信息列表

	CString strMes;
	strMes.Format(_T("当前选中端口号为：%d!"), iPortID);
	DebugMesBox(strMes);

	return iPortID;
}

//枚举所有端口
LPTSTR  EnumerateAllPort()
{
	TCHAR* Port = new TCHAR[1024];
	ZeroMemory(Port, 1024);
	LPTSTR lpPos = Port;

	BOOL bEmuProt = FALSE;
	DWORD dwBufsize = 0;
	DWORD dwNeedSize = 0;
	DWORD dwPortNum = 0;

	bEmuProt = EnumPorts(NULL, 1, NULL, dwBufsize, &dwNeedSize, &dwPortNum);
	DWORD dwErr = GetLastError();
	if (!bEmuProt && dwErr == ERROR_INSUFFICIENT_BUFFER)
	{
		BYTE* lpPortBuf = new BYTE[dwNeedSize];
		ZeroMemory(lpPortBuf, dwNeedSize);

		bEmuProt = EnumPorts(NULL, 1, lpPortBuf, dwNeedSize, &dwNeedSize, &dwPortNum);

		if (bEmuProt)
		{
			PORT_INFO_1* lportinfo = (PORT_INFO_1*)lpPortBuf;

			// 初始化端口列表
			for (DWORD i = 0; i < dwPortNum; i++)
			{
				int iLeng = wcslen(lportinfo->pName);
				memcpy(lpPos, lportinfo->pName, iLeng * 2);
				for (int i = 0; i < iLeng; i++)
				{
					lpPos++;
				}
				memcpy(lpPos, _T(";"), 1);
				lpPos++;

				lportinfo++;
			}
		}
		else
		{
			GetErrAndLog(_T("EnumPorts"));
		}

		delete[]lpPortBuf;
		lpPortBuf = NULL;
	}
	else
	{
		GetErrAndLog(_T("GetLastError: Error 122"));
	}
	return Port;
}

// // // // 复制字体至系统CFont路径下
// // // BOOL  CopyFont()
// // // {
// // // 	BOOL bReturn = FALSE;
// // // 	CString FolderPath = _T("");
// // // 	CString strAppPath = _T("");
// // // 	GetModuleFileName(NULL, strAppPath.GetBuffer(_MAX_PATH), _MAX_PATH);
// // // 	strAppPath.ReleaseBuffer();
// // // 	int nPos = strAppPath.ReverseFind('\\');
// // // 	strAppPath = strAppPath.Left(nPos + 1);
// // // 	strAppPath += _T("字体\\");  //加上txt文件
// // // 	FolderPath = strAppPath;
// // //
// // // 	TCHAR dirCFontPath[BUFSIZE] = { 0 }; //存放系统window\CFont的路径
// // // 	//获取系统ProgarmFile路径
// // // 	SHGetSpecialFolderPath(NULL, dirCFontPath, CSIDL_FONTS, FALSE);
// // // 	lstrcat(dirCFontPath, _T("\\"));
// // // 	//直接COPY当前目录文件，所有文件覆盖
// // // 	bReturn = FindFilesAndCopyToSys(NULL, FolderPath, dirCFontPath,false);
// // //
// // // 	return bReturn;
// // // }
// // //
// // //关闭服务
// // BOOL  CloseSampleService(LPTSTR Service)
// // {
// // 	BOOL bRet = TRUE;
// // 	SERVICE_STATUS status;
// // 	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);   // 打开服务控制管
// // 	//理数据库，并返回服务控制管理数据库的句柄
// // 	if (schSCManager == NULL)
// // 	{
// // 		GetErrAndLog(_T("OpenSCManager"), TRUE);
// // 		return FALSE;
// // 	}
// // 	SC_HANDLE schService = OpenService(schSCManager, Service,
// // 		SERVICE_ALL_ACCESS | DELETE);   // 获得服务句柄
// // 	if (schService == NULL)
// // 	{
// // 		GetErrAndLog(_T("OpenService"), TRUE);
// // 		return FALSE;
// // 	}
// // 	QueryServiceStatus(schService, &status);  // 获得服务的当前状态
// // 	if (status.dwCurrentState != SERVICE_STOPPED)   // 如果服务不处于停止状态，则将其状态设置为	停止状态
// // 	{
// // 		bRet = ControlService(schService, SERVICE_CONTROL_STOP, &status);
// // 		if (!bRet)
// // 		{
// // 			GetErrAndLog(_T("ControlService"), TRUE);
// // 			return FALSE;
// // 		}
// // 	}
// // 	//DeleteService(schService);    // 删除服务
// // 	CloseServiceHandle(schSCManager);  // 关闭服务句柄
// // 	CloseServiceHandle(schService);
// // 	return bRet;
// // }
// //
// //启动服务
// BOOL  StartSampleService(LPTSTR Service)
// {
// 	BOOL bRet = TRUE;
//
// 	SERVICE_STATUS status;
// 	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);   // 打开服务控制管
// 	//理数据库，并返回服务控制管理数据库的句柄
// 	if (schSCManager == NULL)
// 	{
// 		GetErrAndLog(_T("OpenSCManager"), TRUE);
// 		return FALSE;
// 	}
// 	SC_HANDLE schService = OpenService(schSCManager, Service,
// 		SERVICE_ALL_ACCESS | DELETE);    // 获得服务句柄
// 	if (schService == NULL)
// 	{
// 		GetErrAndLog(_T("OpenService"), TRUE);
// 		return FALSE;
// 	}
// 	QueryServiceStatus(schService, &status);   // 获得服务的当前状态
//
// 	//if (status.dwCurrentState != SERVICE_RUNNING)   // 如果服务不处于开启状态，则将其状态设置为启动
// 	//	//状态
// 	//{
// 	//	//bRet = StartService(schService, 0, NULL);   //启动服务
// 	//	status.dwCurrentState = SERVICE_RUNNING;
// 	//	bRet = ControlService(schService, SERVICE_CONTROL_STOP, &status);
// 	//		//SetServiceStatus(schService, &status);
// 	//	GetErrAndLog(_T("ControlService"), TRUE);
// 	//}
// 	//
//
// 	if (status.dwCurrentState == SERVICE_STOPPED)
// 	{
// 		// 启动服务
// 		if (::StartService(schService, NULL, NULL) == FALSE)
// 		{
// 			GetErrAndLog(_T("StartService"), TRUE);
// 			return FALSE;
// 		}
// 		// 等待服务启动
// 		while (::QueryServiceStatus(schService, &status) == TRUE)
// 		{
// 			::Sleep(status.dwWaitHint);
// 			if (status.dwCurrentState == SERVICE_RUNNING)
// 			{
// 				return TRUE;
// 			}
// 			//CString str;
// 			//str.Format(_T("%d"), status.dwCurrentState);
// 			//GetErrAndLog(_T("等待服务启动")+str);
// 		}
// 	}
// 	CloseServiceHandle(schSCManager);   // 关闭服务句柄
// 	CloseServiceHandle(schService);
// 	return bRet;
// }
//
// //运行卸载程序
// BOOL  ShellUninstallExe(LPTSTR AppPath )
// {
// 	if (!lstrcmp(AppPath,_T("")))
// 	{
// 		TCHAR srcUninstall[BUFSIZE] = { 0 }; //存放卸载程序源路径
//
// 		//修改使用GetModuleFileName获取当前路径，防止路径改变时，出现获取不正常情况
// 		CString strAppName;
// 		GetModuleFileName(NULL, strAppName.GetBuffer(_MAX_PATH), _MAX_PATH);
// 		//在其后面加上ini文件名
// 		strAppName.ReleaseBuffer();
// 		int nPos = strAppName.ReverseFind('\\');
// 		strAppName = strAppName.Left(nPos);
//
//
// 		TCHAR* byCurPath = new TCHAR[_MAX_PATH];
// 		ZeroMemory(byCurPath, _MAX_PATH);
// 		lstrcpy(byCurPath, strAppName);
// 		_tcscpy_s(srcUninstall, byCurPath);
// 		lstrcat(srcUninstall, _T("\\"));
// 		lstrcat(srcUninstall, IDS_UninstallPath);
// 		lstrcat(srcUninstall, _T("\\"));
// 		lstrcat(srcUninstall, IDS_UninstallName);
//
// 		AppPath = srcUninstall;
// 		delete[] byCurPath;
// 		byCurPath = NULL;
// 	}
//
// 	ShellExecute(NULL, _T("open"), AppPath, NULL, NULL, SW_SHOW);
// 	return true;
// }

//扫描设备管理器
void  ScanForHardwareChanges1()
{
	DEVINST     devInst;
	CONFIGRET   status;

	//
	// Get the root devnode.
	//
	status = CM_Locate_DevNode(&devInst, NULL, CM_LOCATE_DEVNODE_NORMAL);

	if (status != CR_SUCCESS) {
		//格式化字符串
		CString strError;
		strError.Format(_T("CM_Locate_DevNode failed: %x\n"), status);
		GetErrAndLog(strError);
	}

	status = CM_Reenumerate_DevNode(devInst, 0);

	if (status != CR_SUCCESS) {
		CString strError;
		strError.Format(_T("CM_Reenumerate_DevNode failed: %x\n"), status);
		GetErrAndLog(strError);
		//GetErrAndLog(_T("CM_Reenumerate_DevNode failed: %x\n"), status);
	}
}

// BOOL  ChangePrinterHighAttributes(LPCTSTR lpDeviceName, bool bOpen)
// {
// 	bool bRet = true;
// 	CRegKey SettingKey;
// 	CString szPrinterModel;
// 	szPrinterModel = lpDeviceName;
//
// 	CString szSubKey;
// 	szSubKey = _T("SYSTEM\\CurrentControlSet\\Control\\Print\\Printers\\");
// 	szSubKey += lpDeviceName;
// 	szSubKey += _T("\\DsSpooler");
//
// 	CString strValueName = _T("printSpooling");
//
// 	if (SettingKey.Open(HKEY_LOCAL_MACHINE, szSubKey) != ERROR_SUCCESS)
// 	{
// 		GetErrAndLog(_T("打开注册表默认设置项失败！"));
// 		return FALSE;
// 	}
//
// 	//if (bOpen)
// 	//{
// 	//	bRet = SettingKey.SetDWORDValue(strValueName, 2112);
// 	//}
// 	//else
// 	//{
// 	//	bRet = SettingKey.SetDWORDValue(strValueName, 64);
// 	//}
//
// 	bRet = SettingKey.SetStringValue(strValueName, _T("PrintDirect"));
//
// 	if (bRet)
// 	{
// 		return FALSE;
// 	}
//
// 	CString szSubKey1;
// 	szSubKey1 = _T("SYSTEM\\CurrentControlSet\\Control\\Print\\Printers\\");
// 	szSubKey1 += lpDeviceName;
//
// 	CString strValueName1 = _T("Attributes");
//
// 	if (SettingKey.Open(HKEY_LOCAL_MACHINE, szSubKey1) != ERROR_SUCCESS)
// 	{
// 		GetErrAndLog(_T("打开注册表默认设置项失败！"));
// 		return FALSE;
// 	}
// 	//if (bOpen)
// 	//{
// 	//	bRet = SettingKey.SetDWORDValue(strValueName, 2112);
// 	//}
// 	//else
// 	//{
// 	//	bRet = SettingKey.SetDWORDValue(strValueName, 64);
// 	//}
// 	DWORD ret = 0;
// 	SettingKey.QueryDWORDValue(strValueName1, ret);
//
// 	ret = (ret & 0xfffffff0) | 0x2;
//
// 	bRet = SettingKey.SetDWORDValue(strValueName1, ret);
//
// 	if (bRet)
// 	{
// 		return false;
// 	}
//
// 	RetBootService(_T("Spooler"));
//
//
// 	return bRet;
// }
//

//枚举所有打印机驱动
int	 EnumAllPrintDriver(DRIVER_INFO_6*& die, DWORD& dSize)
{
	int iReturn = -1;
	dSize = 0;
	BOOL bRet = FALSE;

	DRIVER_INFO_6* pLevel1 = NULL;
	DWORD dwNeeded = 0;
	DWORD dwReceived = 0;
	DWORD dwError = 0;
	int nRet = 0;
	nRet = EnumPrinterDrivers(NULL, // local machine
		NULL, // current environment
		6, // level 1, whatever that means
		(LPBYTE)pLevel1,
		0,
		&dwNeeded,
		&dwReceived);
	if (!nRet)
	{
		dwError = GetLastError();
		//判断错误是否为内存不足
		if (dwError == 122)
		{
			DRIVER_INFO_6* pLevel2 = new DRIVER_INFO_6[dwNeeded];

			nRet = EnumPrinterDrivers(NULL, // local machine
				NULL, // current environment
				6, // level 1, whatever that means
				(LPBYTE)pLevel2,
				dwNeeded,
				&dwNeeded,
				&dwReceived);

			if (nRet)
			{
				bRet = TRUE;
				die = pLevel2;
				iReturn = dwReceived;
				dSize = dwNeeded;
			}
			else
			{
				delete[] pLevel2;
				pLevel2 = NULL;

				GetErrAndLog(_T("EnumPrinterDrivers"));
			}
		}
	}

	return iReturn;
}