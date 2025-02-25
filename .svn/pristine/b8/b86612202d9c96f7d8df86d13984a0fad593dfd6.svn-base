// AutoRun.h

#pragma once

#include "ExportHead.h"

typedef  struct  //UIDPID
{
	CString	Pnp;
	CString	Usb;
}MyID;

//开始安装驱动，成功返回TRUE，否则FALSE
//BOOL StartAddPrinter(HWND, CString Seriver, CString DeviceName, CString Port, CString GPD, int iMark);
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
int CopyFilesToSys(HWND hwnd, CString Seriver, CString Model, CString GPD, LPCTSTR PointMark, BOOL OsX64, InstallFile File, LPCTSTR infName);

//直接拷贝到系统打印机文件夹路径下
//BOOL FindFilesAndCopyToSys(HWND hWnd, LPCTSTR lpBootFolderPath, LPCTSTR lpPrinterDriverPath, CString DeviceName, CString GPD, InstallFile File);
BOOL FindFilesAndCopyToSys(HWND hWnd, LPCTSTR lpBootFolderPath, LPCTSTR lpPrinterDriverPath, const CString& DeviceName, const CString& GPD, const InstallFile& File, BOOL OsX64);
//复制文件至直接目录，有了就覆盖，没有就复制
BOOL MyCopyFile(HWND hWnd, LPCTSTR lpSrcDir, LPCTSTR lpDestDir);
// 判断两个文件是否完全相同 ,相同返回true
BOOL IsSameFile(LPCTSTR lpSrcFilename, LPCTSTR lpDirFilename);

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
int AddPrinterToSys(LPCTSTR InstallName, CString Port, CString GPD, InstallFile File, BOOL OsX64);

//// 添加注册表默认设置
///***************
//参数：
//InstallName ： 安装的驱动名称
//错误代码：
//成功：TRUE
//失败：FALSE
//***************/
//BOOL AddRegValue(LPCTSTR InstallName);

//设置打印机为默认打印机
/***************
参数：
InstallName ： 安装的驱动名称
***************/
BOOL  SetPrinterDefault(LPCTSTR  lpDeviceName);

//直接拷贝到系统打印机文件夹路径下，重载函数，其中文件全部覆盖,并确认是否创建快捷菜单链接
//BOOL FindFilesAndCopyToSys(HWND hWnd, LPCTSTR lpSrcDir, LPCTSTR lpDestDir, BOOL CreateLink);

// /*新增机型需要函数*/
// //如果是新增机型，先将Inf拷贝至临时文件夹，添加新增机型信息，并签名，返回新的新的Inf完成路径
// /***************
// 参数：
// Model：打印机型号
// InstallName ： 安装的驱动名称
// PointMark ： 多少针标识
// GPD ： GPD文件名称
// OsX64 ： 是否为X64系统
// MFG ：产家信息，例如START 用来生成pnpid
// 返回值：
// 新的inf的路径：
// ***************/
// LPCTSTR ModNewPrintInf(LPCTSTR Model, LPCTSTR InstallName, LPCTSTR PointMark, LPCTSTR GPD, BOOL OsX64, LPCTSTR  MFG, LPCTSTR infName);
// 传入文件夹路径，遍历里面所有文件
//void TraversaFolder(CString FolderPath,LPCTSTR Model, LPCTSTR InstallName, int iPointMark, LPCTSTR GPD, BOOL OsX64, LPCTSTR  MFG);
//如果是新增机型，安装成功后需要修改inf文件
// BOOL ModInfFile(CString FilePath, LPCTSTR Model, LPCTSTR InstallName, CString GPD, LPCTSTR  MFG);
//// 根据型号获取ID
//MyID GetPnpUsbId(CString MFG, CString Print);
////拷贝inf及gpd文件至临时文件目录，
//CString CopyInfToTemp(CString oldInfPath, BOOL bIs64);
////签名Inf文件
//BOOL SignInf(CString infDirPath, CString infName, BOOL bIs64);
////获取inf中Cat文件路径
//CString GetCatPath(CString infPath, CString infName);
//直接删除到路径下所有文件
//BOOL DeleteAllFiles(LPCTSTR lpBootFolderPath);
//判断文件是否存在
//BOOL IsExistFile(CString FilePath);
//查看当前进程是否正在运行
//BOOL _FindProcess(CString strProcessName, CArray<DWORD, DWORD>& aPid);

//LPTSTR CreateNewPath(HWND hWnd);

//删除此名字的线程
//BOOL _DeleteProcess(CString strProcessName);

//读取注册表信息
//bool RegistryGetValue(HKEY hk, const TCHAR * pszKey, const TCHAR * pszValue, DWORD dwType, LPBYTE data, DWORD dwSize);