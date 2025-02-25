//#pragma once

#include "stdafx.h"
#include <afxwin.h>
#include "AutoRun.h"
#include <Winspool.h>
#include <SetupAPI.h>
#include "InfFileHelper.h"
#include "DebugStuff.h"
#include "UDefault.h"
#include "GetSysVersion.h"
#include "StrTranslate.h"
#include "DriversIni.h"
#pragma comment(lib,"newdev.lib")
#include "OnlineRun.h"
#include "tlhelp32.h"
#include "SetupDlg.h"

extern CString g_MFG;
extern bool g_bStopPrintSpooler;

//拷贝文件至打印机目录
/***************
参数：
DlghWnd ： 对话框句柄
Seriver ： 系列
DeviceName   ： 型号
GPD ：GPD的名称
PointMark  ：针数标识 【24针：24Pin，12针：12Pin，9针：9Pin，热敏：Thermal，热转印：ThermalTurn】
OsX64 ：是否为64位系统
infName ：inf名称
错误代码：
0：代表成功
1：未能获取到打印机路径
2：拷贝文件失败
3：安装inf文件失败
***************/
int CopyFilesToSys(HWND hwnd, CString Seriver, CString Model, CString GPD, LPCTSTR PointMark, BOOL OsX64, InstallFile File, LPCTSTR infName)
{
	int iRet = 0;
	//  根据系统判断标识加载环境变量参数
	LPTSTR lpEnvironment = NULL;

	// 获取系统驱动文件安装目标文件夹路径
	DWORD dwPathLen;
	BYTE  byDriverPath[BUFSIZE] = { 0 };
	//判断指定系统中包含了打印机驱动程序的目录是什么
	BOOL bCpyRel = GetPrinterDriverDirectory(NULL, lpEnvironment, 1, byDriverPath, BUFSIZE, &dwPathLen);

	CString strMes;

	if (!bCpyRel)       // 成功获取系统驱动文件夹路径
	{
		return  1;
	}
	else
	{
		strMes.Format(_T("The PrinterDriverPath is : %s"), byDriverPath);

		//修改使用GetModuleFileName获取当前路径，防止路径改变时，出现获取不正常情况
		CString strAppName;
		GetModuleFileName(NULL, strAppName.GetBuffer(_MAX_PATH), _MAX_PATH);
		//在其后面加上ini文件名
		strAppName.ReleaseBuffer();
		int nPos = strAppName.ReverseFind('\\');
		strAppName = strAppName.Left(nPos);

		TCHAR* byCurPath = new TCHAR[_MAX_PATH];
		ZeroMemory(byCurPath, _MAX_PATH);
		lstrcpy(byCurPath, strAppName);

		DebugMesBox(byCurPath);
		//	/************************************************************************/
		//	/*        拷贝驱动文件到系统驱动文件夹路径下                             */
		//	/************************************************************************/
		BOOL bCpy = FALSE;

		TCHAR dirFilePath[BUFSIZE] = { 0 };			//驱动文件安装路径
		TCHAR srcPublicFilePath[BUFSIZE] = { 0 };	//驱动文件源路径
		//选择安装的多少针的
		TCHAR lpFolder[256] = { 0 };  //64还是32为路径
		lstrcat(lpFolder, _T("\\"));
		lstrcat(lpFolder, PointMark);
		lstrcpy(srcPublicFilePath, byCurPath);
		lstrcat(srcPublicFilePath, lpFolder);

		////判断是64位系统还是32位
		//if (OsX64)
		//{
		//	lstrcat(srcPublicFilePath, _T("\\X64\\"));
		//}
		//else
		//{
		//	lstrcat(srcPublicFilePath, _T("\\i386\\"));
		//}
		//DebugMesBox(srcPublicFilePath);

		lstrcpy(dirFilePath, (LPCTSTR)byDriverPath);
		lstrcat(dirFilePath, _T("\\"));

		//拷贝安装文件
		bCpy = FindFilesAndCopyToSys(hwnd, srcPublicFilePath, dirFilePath, Model, GPD, File, OsX64);
		if (!bCpy)
		{
			DebugMesBox(_T("Find Files and Copy  To Sysdir Fail! "));

			delete[] byCurPath;
			byCurPath = NULL;

			return 2;
		}

		// 拷贝inf文件到系统路径下
		TCHAR OEMSourceMediaLocation[BUFSIZE] = { 0 }; //存放inf根目录路径
		lstrcpy(OEMSourceMediaLocation, byCurPath);

		TCHAR InfPath[BUFSIZE] = { 0 };  //存放inf完整路径
		lstrcat(OEMSourceMediaLocation, lpFolder);

		//if (OsX64)
		//{
		//	//24pin
		//	lstrcat(OEMSourceMediaLocation, _T("\\X64\\"));
		//	lstrcpy(InfPath, OEMSourceMediaLocation);
		//}
		//else
		//{
		//	//24pin
		//	lstrcat(OEMSourceMediaLocation, _T("\\i386\\"));
		//	lstrcpy(InfPath, OEMSourceMediaLocation);
		//}

		lstrcpy(InfPath, OEMSourceMediaLocation);
		lstrcat(InfPath, _T("\\"));
		lstrcat(InfPath, infName);

		//拷贝inf文件
		bCpy = SetupCopyOEMInf(InfPath, OEMSourceMediaLocation, SPOST_PATH, NULL, NULL, 0, NULL, NULL);
		if (!bCpy)
		{
			if (GetLastError() == ERROR_NOT_FOUND)	 	//1168
			{
				//找不到元素

				bCpy = SetupCopyOEMInf(InfPath, OEMSourceMediaLocation, SPOST_PATH, NULL, NULL, 0, NULL, NULL);
				if (!bCpy)
				{
					GetErrAndLog(_T("SetupCopyOEMInf"));

					delete[] byCurPath;
					byCurPath = NULL;
					return 3;
				}
			}
			else
			{
				GetErrAndLog(_T("SetupCopyOEMInf"));
				delete[] byCurPath;
				byCurPath = NULL;
				return 3;
			}
		}
	}
	return iRet;
}

// 递归查找并拷贝指定路径下的所有文件（包括子文件夹下）
BOOL FindFilesAndCopyToSys(HWND hWnd, LPCTSTR lpBootFolderPath, LPCTSTR lpPrinterDriverPath, const CString& DeviceName, const CString& GPD, const InstallFile& File, BOOL OsX64)
{
	// 关闭打印服务
	g_bStopPrintSpooler = StopPrintSpoolerService();
	BOOL bResult = FALSE; // 初始化结果标志为假

	if (g_bStopPrintSpooler)
	{
		// 等待1秒
		Sleep(1500);

		CFileFind finder; // 创建文件查找对象
		CString strWildcard(lpBootFolderPath);
		strWildcard.Append(_T("\\*.*")); // 构造搜索模式字符串，表示搜索所有文件

		BOOL bWorking = finder.FindFile(strWildcard); // 开始搜索文件
		while (bWorking)
		{
			bWorking = finder.FindNextFile(); // 查找下一个文件或目录

			if (finder.IsDots())
				continue; // 如果是当前目录或上级目录的标识，则跳过

			if (finder.IsDirectory()) // 如果找到的是目录
			{
				// 获取子目录的名称
				CString newDirName = finder.GetFileName();

				// 根据传入的系统标识OsX64，判断要跳转到32bit目录下还是64bit目录下
				if (OsX64)
				{
					if (newDirName == _T("32bit")) // 如果是32bit目录
						continue; // 跳过
				}
				else
				{
					if (newDirName == _T("64bit")) // 如果是64bit目录
						continue; // 跳过
				}

				// 构造子目录的完整路径
				CString newDirPath = finder.GetFilePath(); // 获取子目录的完整路径

				//递归调用以处理子目录
				bResult |= FindFilesAndCopyToSys(hWnd, newDirPath, lpPrinterDriverPath, DeviceName, GPD, File, OsX64);
			}
			else // 如果找到的是文件
			{
				CString strFilePath = finder.GetFilePath(); // 获取文件的完整路径
				CString strFileName = finder.GetFileName(); // 获取文件名

				// 构造新文件的完整路径
				CString newFilePath = lpPrinterDriverPath;
				if (newFilePath.Right(1) != _T("\\")) newFilePath += _T("\\");
				newFilePath += strFileName;

				// 要拷贝到的目标文件的另一个路径lpPrinterDriverPath+\\3
				CString newFilePath3 = lpPrinterDriverPath;
				if (newFilePath3.Right(1) != _T("\\")) newFilePath3 += _T("\\");
				newFilePath3 += _T("3\\");
				newFilePath3 += strFileName;

				// 判断是否是需要拷贝的文件
				CString strMustCopyFiles = File.MustCopy + GPD + _T(";");
				CString strCoverFiles = File.CoverFile + GPD + _T(";");

				strMustCopyFiles.MakeLower();
				strCoverFiles.MakeLower();
				CString lowerFileName = strFileName;
				lowerFileName.MakeLower();

				if (strMustCopyFiles.Find(lowerFileName + _T(";")) != -1) // 如果文件在必须拷贝的列表中
				{
					BOOL bCover = strCoverFiles.Find(lowerFileName + _T(";")) != -1; // 是否需要覆盖
					if (bCover)
					{
						// 如果需要覆盖，则调用覆盖拷贝
						bResult |= MyCopyFile(hWnd, strFilePath, newFilePath); // 假定MyCopyFile处理覆盖逻辑
						bResult |= MyCopyFile(hWnd, strFilePath, newFilePath3); // 假定MyCopyFile处理覆盖逻辑
					}
					else
					{
						// 如果不需要覆盖，直接调用CopyFile
						bResult |= CopyFile(strFilePath, newFilePath, TRUE); // TRUE表示如果目标文件存在则不覆盖
						bResult |= CopyFile(strFilePath, newFilePath3, TRUE); // TRUE表示如果目标文件存在则不覆盖
					}
				}
			}
		}
		finder.Close(); // 关闭查找器

		// 再次启动打印服务
		if (StartPrintSpoolerService())
		{
			g_bStopPrintSpooler = FALSE; // 重置打印服务停止标志
			// 等待1秒
			Sleep(1500);
		}
		else {
			// 未能启动打印服务
			CString strMes;
			strMes.Format(_T("Failed to start the print spooler service!"));
			LogOutput(strMes);
		}
	}
	else
	{
		// 未能停止打印服务
		CString strMes;
		strMes.Format(_T("Failed to stop the print spooler service!"));
		LogOutput(strMes);
	}

	return bResult; // 返回操作结果
}

BOOL  MyCopyFile(HWND hWnd, LPCTSTR lpSrcDir, LPCTSTR lpDestDir)
{ // 0
	DWORD dwRet = 0;		 // 用于存放文件属性变量	05-08-04
	BOOL  result = TRUE;			// 拷贝成功与否标志符

#ifdef _DEBUG
	// 将拷贝文件的信息写入日志文件
	CString strMes;
	strMes.Format(_T("CopyFile: %s to %s"), lpSrcDir, lpDestDir);
	LogOutput(strMes);
#endif

	// 覆盖拷贝文件
	result = CopyFile(lpSrcDir, lpDestDir, FALSE);

	// 无法覆盖拷贝，判断原因是因为目标文件为只读属性还是被其他进程占用
	if (!result)
	{
		DWORD dwErr = GetLastError();
		if (ERROR_ACCESS_DENIED == dwErr
			|| ERROR_SHARING_VIOLATION == dwErr)
		{
			// 尝试改变文件的只读属性后再拷贝
			dwRet = GetFileAttributes(lpDestDir);
			dwRet = dwRet & ~FILE_ATTRIBUTE_READONLY;
			SetFileAttributes(lpDestDir, dwRet);
			result = CopyFile(lpSrcDir, lpDestDir, FALSE);
			if (!result)
			{
				CString strMes;
				strMes.Format(_T("CopyFile: %s to %s"), lpSrcDir, lpDestDir);

				GetErrAndLog(strMes);
			}
		}
	}

	return result;
}

// 判断两个文件是否完全相同 ,相同返回true
BOOL IsSameFile(LPCTSTR lpSrcFilename, LPCTSTR lpDirFilename)
{
	BOOL bIsSame = FALSE;

	CString strMes;
	CFile fdirFile;
	CString strFileBuf1, strFileBuf2;

	if (fdirFile.Open(lpDirFilename, CFile::modeRead | CFile::shareDenyNone))
	{
		strMes.Format(_T("the dirFileFullPath is %s!"), lpDirFilename);

		ULONG uFileSize1 = (ULONG)fdirFile.GetLength();

		char* lpFileDatabuf1 = new char[uFileSize1 + 1];

		ZeroMemory(lpFileDatabuf1, uFileSize1 + 1);

		fdirFile.Read(lpFileDatabuf1, uFileSize1);

		// 关闭打开的目标文件
		fdirFile.Close();

		CFile fsrcFile;
		if (fsrcFile.Open(lpSrcFilename, CFile::modeRead))
		{ // 3
			strMes.Format(_T("the srcFileFullPath is %s!"), lpSrcFilename);
			//DebugMesBox(strMes);

			ULONG uFileSize2 = (ULONG)fsrcFile.GetLength();

			char* lpFileDatabuf2 = new char[uFileSize2 + 1];

			ZeroMemory(lpFileDatabuf2, uFileSize2 + 1);

			fsrcFile.Read(lpFileDatabuf2, uFileSize2);

			// 关闭打开的目标文件
			fsrcFile.Close();

			// 比较文件
			if (uFileSize1 == uFileSize2)
			{
				int iRet = ::memcmp(lpFileDatabuf1, lpFileDatabuf2, uFileSize1);

				if (iRet == 0)
				{
					bIsSame = TRUE;
				}
			}
			delete[] lpFileDatabuf2;
		}
		else
		{
			strMes.Format(_T("Open OriFile:%s"), lpSrcFilename);
			DumpErrorMes(strMes);
		}
		delete[] lpFileDatabuf1;
	}
	else
	{
		strMes.Format(_T("Open DirFile:%s"), lpDirFilename);
		DumpErrorMes(strMes);
	}

	return bIsSame;
}

//增加打印机到PC机
/***************
参数：
InstallName ： 安装的驱动名称
Port ： 端口号
GPD ：GPD文件名称
File ：文件关系
错误代码：
0：代表成功
1：找不到路径
2：增加打印机驱动失败
3：增加打印机失败
***************/
int AddPrinterToSys(LPCTSTR InstallName, CString Port, CString GPD, InstallFile File, BOOL OsX64)
{
	int iRet = 0;
	LPTSTR lpEnvironment = NULL;

	// 获得系统打印机驱动文件安装路径  //如C:\Windows\System32\spool\drivers\x64
	DWORD dwPathLen;
	BYTE  byDriverPath[BUFSIZE] = { 0 };
	BOOL bCpyRel = GetPrinterDriverDirectory(NULL, lpEnvironment, 1, byDriverPath, BUFSIZE, &dwPathLen);

	CString strMes = _T("");  //用来转为CString，调试弹出信息

	if (bCpyRel)
	{
		DRIVER_INFO_6 di6;  //存放  //   2K~Win7 OS   打印机驱动程序的信息。

		PRINTER_INFO_2 pi2;

		HANDLE	hPrinter;  //打印机句柄
		CString DeviceName = InstallName;

		LPTSTR lpDeviceFile = (LPTSTR)InstallName;//存放驱动名字
		//DebugMesBox(lpDeviceFile);
		LPTSTR lpDataType1 = IDS_DataType1;
		//DebugMesBox(lpDataType1);

		LPTSTR lpManufacturer = g_MFG.GetBuffer();  //生产产家
		//DebugMesBox(lpManufacturer);
		LPTSTR lpPortName = Port.GetBuffer();  //端口名字
		//DebugMesBox(lpPortName);

		LPTSTR lpPrintProcessor = IDS_PrintProcessor;   //添加打印机使用，指定所使用的打印机的打印处理器的名称
		//DebugMesBox(lpPrintProcessor);

		TCHAR strBuf[BUFSIZE] = { 0 };							// 创建依赖文件字符串缓存  add by hl 2011.09.07
		LPTSTR pos = strBuf;
		int i = 0;

		int iPrePos = 0, iNextPos;

		//DebugMesBox(_T("准备拷贝文件到系统目录！"));

		//拷贝驱动程序文件完整路径
		TCHAR lpDriverPath[BUFSIZE] = { 0 };
		lstrcpy(lpDriverPath, (LPCTSTR)byDriverPath);
		lstrcat(lpDriverPath, _T("\\"));
		lstrcat(lpDriverPath, IDS_DriverFile);

		TCHAR* lpExistFilename = new TCHAR[BUFSIZE];
		ZeroMemory(lpExistFilename, BUFSIZE);
		lstrcpy(lpExistFilename, (LPCTSTR)byDriverPath);
		lstrcat(lpExistFilename, _T("\\3\\"));
		lstrcat(lpExistFilename, IDS_DriverFile);

		CopyFile(lpExistFilename, lpDriverPath, TRUE);
		ZeroMemory(lpExistFilename, BUFSIZE);

		//拷贝文件名或完整路径和文件名的文件，其中包含驱动程序数据
		TCHAR lpDataFilePath[BUFSIZE] = { 0 };
		lstrcpy(lpDataFilePath, (LPCTSTR)byDriverPath);
		lstrcat(lpDataFilePath, _T("\\"));
		//根据选择机型剥离对应的GPD文件名代码
		lstrcat(lpDataFilePath, GPD.GetBuffer());
		//strMes.Format(_T("the DataFilePath path is: %s!"), lpDataFilePath);
		//DebugMesBox(strMes);

		//动态链接库的路径
		TCHAR lpConfigFilePath[BUFSIZE] = { 0 };
		lstrcpy(lpConfigFilePath, (LPCTSTR)byDriverPath);
		lstrcat(lpConfigFilePath, _T("\\"));
		lstrcat(lpConfigFilePath, IDS_ConfigFile);

		lstrcpy(lpExistFilename, (LPCTSTR)byDriverPath);
		lstrcat(lpExistFilename, _T("\\3\\"));
		lstrcat(lpExistFilename, IDS_ConfigFile);

		CopyFile(lpExistFilename, lpConfigFilePath, TRUE);
		ZeroMemory(lpExistFilename, BUFSIZE);
		//strMes.Format(_T("the ConfigFilePath path is: %s!"), lpConfigFilePath);
		//DebugMesBox(strMes);

		//帮助文件的路径
		TCHAR lpHelpsFilePath[BUFSIZE] = { 0 };
		lstrcpy(lpHelpsFilePath, (LPCTSTR)byDriverPath);
		lstrcat(lpHelpsFilePath, _T("\\"));
		lstrcat(lpHelpsFilePath, IDS_HelpFile);

		lstrcpy(lpExistFilename, (LPCTSTR)byDriverPath);
		lstrcat(lpExistFilename, _T("\\3\\"));
		lstrcat(lpExistFilename, IDS_HelpFile);

		CopyFile(lpExistFilename, lpHelpsFilePath, TRUE);
		ZeroMemory(lpExistFilename, BUFSIZE);

		// 拷贝系统资源dll文件
		TCHAR lpResFilePath[BUFSIZE] = { 0 };
		lstrcpy(lpResFilePath, (LPCTSTR)byDriverPath);
		lstrcat(lpResFilePath, _T("\\"));
		lstrcat(lpResFilePath, IDS_RESFILE);

		lstrcpy(lpExistFilename, (LPCTSTR)byDriverPath);
		lstrcat(lpExistFilename, _T("\\3\\"));
		lstrcat(lpExistFilename, IDS_RESFILE);

		CopyFile(lpExistFilename, lpResFilePath, TRUE);

		delete[]lpExistFilename;
		lpExistFilename = NULL;

		//strMes.Format(_T("the HelpsFilePath path is: %s!"), lpHelpsFilePath);
		//DebugMesBox(strMes);

		ZeroMemory(&di6, sizeof(DRIVER_INFO_6));
		di6.cVersion = 3;  //写入的操作系统版本
		di6.pName = lpDeviceFile;  //驱动程序的名称
		di6.pEnvironment = lpEnvironment; //系统版本
		di6.pDriverPath = lpDriverPath;  //驱动程序文件完整路径
		di6.pDataFile = lpDataFilePath; //指定文件名或完整路径和文件名的文件，其中包含驱动程序数据
		di6.pConfigFile = lpConfigFilePath;  //动态链接库的完整路径和文件名
		di6.pHelpFile = lpHelpsFilePath; //帮助文件的完整路径和文件名
		di6.pMonitorName = NULL;  //指定语言监视器
		di6.pDefaultDataType = lpDataType1; //指定打印作业的默认数据类型（例如，“EMF”）
		di6.pszMfgName = lpManufacturer; //指向一个空终止字符串，指定制造商的名称。
		di6.pszzPreviousNames = NULL;//指定任何以前的打印机驱动程序的名称与此驱动程序
		di6.pszOEMUrl = NULL;//指定URL的制造商

		CString strTmp;
		CString	strfiles = File.DependentFiles;//依赖文件名称

		while ((iNextPos = strfiles.Find(_T(";"), iPrePos)) != -1)
		{
			strTmp = strfiles.Mid(iPrePos, iNextPos - iPrePos);
			iPrePos = iNextPos + 1;

			lstrcat(pos, strTmp.GetBuffer());
			pos += strTmp.GetLength();
			pos++;
		}

		di6.pDependentFiles = strBuf; //指定驱动程序依赖于文件。每个文件名是由一个反斜杠和一个空字符
		strMes.Format(_T("di6.pName	= %s ; 	 di6.pEnvironment	 =  %s; di6.pDriverPath = %s ;  di6.pDataFile = %s;  \
							 							di6.pConfigFile	 = %s; di6.pHelpFile = %s;	 di6.pDefaultDataType = %s;	 di6.pszMfgName = %s;"), \
			di6.pName, di6.pEnvironment, di6.pDriverPath, \
			di6.pDataFile, di6.pConfigFile, di6.pHelpFile, \
			di6.pDefaultDataType, di6.pszMfgName);
		DebugMesBox(strMes);

		if (!AddPrinterDriver(NULL, 6, (LPBYTE)&di6))
			//if (!AddPrinterDriverEx(NULL, 6, (LPBYTE)&di6, APD_COPY_ALL_FILES))
		{
			//GetErrAndLog(_T("AddPrinterDriver"),TRUE);
			GetErrAndLog(_T("AddPrinterDriver"));
			return 2;
		}

		//增加打印机
		ZeroMemory(&pi2, sizeof(PRINTER_INFO_2));
		pi2.DefaultPriority = 0;   //指定分配给每个打印作业的缺省优先级值
		pi2.pPrinterName = lpDeviceFile;        //打印机名字
		pi2.pPortName = lpPortName;   //端口名字
		pi2.pDriverName = lpDeviceFile;					//驱动名字		// refix by hl  2011.09.26
		pi2.pPrintProcessor = lpPrintProcessor;  //指定所使用的打印机的打印处理器的名称
		pi2.pDatatype = lpDataType1;   //指定用于记录打印作业的数据类型
		pi2.Attributes = PRINTER_ATTRIBUTE_LOCAL;  //指定打印机属性

		strMes.Format(_T("pi2.pPrinterName = %s ; \
						 						 pi2.pPortName	 =  %s;   \
												 						 pi2.pDriverName = %s ;  \
																		 						 pi2.pPrintProcessor = %s;  \
																								 						 pi2.pDatatype	 = %s; "), \
			pi2.pPrinterName, pi2.pPortName, pi2.pDriverName, \
			pi2.pPrintProcessor, pi2.pDatatype);

		hPrinter = AddPrinter(NULL, 2, (LPBYTE)&pi2);

		if (hPrinter == NULL)
		{
			//GetErrAndLog(_T("AddPrinter"), TRUE);
			GetErrAndLog(_T("AddPrinter"));
			return 3;
		}

		ClosePrinter(hPrinter);
	}
	else
	{
		//GetErrAndLog(_T("无法找到当前打印机驱动安装路径!"));
		GetErrAndLog(_T("无法找到当前打印机驱动安装路径!"));
		return 1;
	}

	return iRet;
}

//设置打印机为默认打印机
/***************
参数：
InstallName ： 安装的驱动名称
错误代码：
成功：TRUE
失败：FALSE
***************/
BOOL SetPrinterDefault(LPCTSTR InstallName)
{
	return SetDefaultPrinter(InstallName);
}

// // 传入文件夹路径，遍历里面所有文件
// void TraversaFolder(CString FolderPath, LPCTSTR Model, LPCTSTR InstallName, int iPointMark, LPCTSTR GPD, BOOL OsX64, LPCTSTR  MFG)
// {
//
// 	CFileFind tempFind;												 //声明一个CFileFind类变量，以用来搜索指定路径下文件
// 	TCHAR tempFileFind[MAX_PATH] = { 0 };			 //用于定义搜索格式
// 	lstrcpy(tempFileFind, FolderPath);
// 	lstrcat(tempFileFind, _T("*.*"));							 //匹配格式为*.*,即该目录下的所有文件
//
// 	BOOL IsFinded = tempFind.FindFile(tempFileFind);		 //查找第一个文件
// 	while (IsFinded)
// 	{ // 1
// 		IsFinded = tempFind.FindNextFile();								 //递归搜索其他的文件
// 		if (!tempFind.IsDots())														//如果不是"."目录
// 		{ //2
// 			TCHAR foundFileName[MAX_PATH] = { 0 };
// 			lstrcpy(foundFileName, tempFind.GetFileName().GetBuffer());
//
// 			if (tempFind.IsDirectory())												//如果是目录，则递归地调用
// 			{ // 3
// 				/*TCHAR tempDir[MAX_PATH] = { 0 };
// 				lstrcpy(tempDir, FolderPath);
// 				lstrcat(tempDir, foundFileName);
// 				lstrcat(tempDir, _T("\\"));
//
// 				TraversaFolder(tempDir, Seriver, lpDeviceName, GPD);*/
// 			} // 3 //
// 			else
// 			{
//
// 				CString  lpNewFilePath;
// 				lpNewFilePath = FolderPath + foundFileName;  //复制完整路径
// 				//是inf文件，调用函数打开
// 				if (lpNewFilePath.Find(_T(".inf")) != -1)
// 				{
// 					//调用函数修改其中inf文件内容
// 					ModInfFile(lpNewFilePath, Model, InstallName, GPD, MFG);
// 				}
//
// 			} // 3 //
// 		} // 2 //
// 	} //1 //
// 	tempFind.Close();
// }

// // //直接拷贝到系统打印机文件夹路径下，重载函数，其中文件全部覆盖
//BOOL FindFilesAndCopyToSys(HWND hWnd, LPCTSTR lpSrcDir, LPCTSTR lpDestDir, BOOL CreateLink)
//{
//	BOOL bResult = FALSE;
//
//
//	CFileFind tempFind;												 //声明一个CFileFind类变量，以用来搜索指定路径下文件
//	TCHAR tempFileFind[MAX_PATH] = { 0 };			 //用于定义搜索格式
//	lstrcpy(tempFileFind, lpSrcDir);
//	lstrcat(tempFileFind, _T("*.*"));							 //匹配格式为*.*,即该目录下的所有文件
//
//	BOOL IsFinded = tempFind.FindFile(tempFileFind);		 //查找第一个文件
//	while (IsFinded)
//	{ // 1
//		IsFinded = tempFind.FindNextFile();								 //递归搜索其他的文件
//		if (!tempFind.IsDots())														//如果不是"."目录
//		{ //2
//			TCHAR foundFileName[MAX_PATH] = { 0 };
//			lstrcpy(foundFileName, tempFind.GetFileName().GetBuffer());
//
//			if (tempFind.IsDirectory())												//如果是目录，则递归地调用
//			{ // 3
//				TCHAR tempDir[MAX_PATH] = { 0 };
//				lstrcpy(tempDir, lpSrcDir);
//				lstrcat(tempDir, foundFileName);
//				lstrcat(tempDir, _T("\\"));
//
//				bResult = FindFilesAndCopyToSys(hWnd, tempDir, lpDestDir, CreateLink);
//			} // 3 //
//			else
//			{	// 3      // 当前是文件，组织全路径
//				CString strFileName = foundFileName;
//				CString lpExistFilename, lpNewFilename;
//				lpExistFilename = lpSrcDir + strFileName;
//				lpNewFilename = lpDestDir + strFileName;
//
//				//判断当前文件是否是要拷贝的文件
//				//CString strSysFiles;  //存放所有要拷贝的文件的名字，用;隔开
//				//CString strCoverFiles;  //存放复制时要覆盖的文件名字
//				//新增机型时候不拷贝cat文件
//				if (lpExistFilename.Find(_T(".cat"))== -1)
//				{
//					bResult = MyCopyFile(hWnd, lpExistFilename, lpNewFilename);
//				}
//				////添加快捷方式
//				//if (bResult && CreateLink)
//				//{
//				//	CCreateStartLink Link;
//				//	Link.createShortcut(lpNewFilename, FALSE);
//				//}
//			} // 3 //
//		} // 2 //
//	} //1 //
//	tempFind.Close();
//
//	return bResult;
//}			 