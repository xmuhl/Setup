#pragma once

#include <WinSpool.h>

struct InstallFile          //预安装时需要的文件关系
{
	//例如 _T("STDNAMES.GPD;UNIRES.DLL;START24RES.DLL;")
	LPTSTR DependentFiles;  //依赖文件的名称
	LPTSTR CoverFile;  //必须覆盖的文件
	LPTSTR MustCopy;  //必须拷贝的文件
};

//获取打印机驱动时的信息
typedef struct
{
	CString strEnvironment;  //系统标识
	CString strName;  //驱动名称
	DWORD dwVersion;  //版本号
}MY_DRIVER_INFO;

//使用预安装模式安装打印机驱动
/***************
参数：
DlghWnd ： 对话框句柄
Seriver ： 系列
Model   ： 型号
InstallName ：安装名称
Port ：端口号
GPD ：GPD的名称
PointMark  ：针数标识 【24针：24Pin，12针：12Pin，9针：9Pin，热敏：Thermal，热转印：ThermalTran】
OsX64 ：是否为64位系统
infName ：安装的驱动的Inf名称
错误代码：
0：代表成功
1：拒绝拷贝文件
2：安装inf文件失败
3：增加打印机驱动失败
4：写入注册表失败
5：增加打印机失败
6：设为默认打印机失败
***************/
int   PreInstalled(HWND DlghWnd, LPCTSTR Seriver, LPCTSTR Model, LPCTSTR InstallName, LPCTSTR Port, LPCTSTR GPD, LPCTSTR PointMark, BOOL OsX64, InstallFile File, LPCTSTR infName);

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
BOOL   OnlineInstalled(HWND DlghWnd, LPCTSTR Model, LPCTSTR InstallName, LPCTSTR PointMark, LPCTSTR GPD, BOOL OsX64, LPCTSTR  MFG, BOOL bNewModel, LPCTSTR infName);
//拷贝工具及文档，
/***************
参数：
ConfigFile ： 安装时选择的备选附件，工具集，手册等，每个文件以封号区分 例如：_T("工具集;用户手册;")
BootFolder ： 根目录文件夹名称
错误代码：
成功：TRUE
失败：FALSE
***************/
//BOOL   CopyToolAndDoc(LPCTSTR FileName, LPCTSTR BootFolder);
// // //判断.NET Framework 4.0是否安装
// // /***************
// // 返回值：
// // 已安装：TRUE
// // 未安装：FALSE
// // ***************/
// // BOOL  IsInstallNet40();
// //
// //运行安装.net4.0
// /***************
// 参数：
// Weblink ：网页链接，目前放在  http://shidaapp.duapp.com/Down/Microsoft.NET.exe"
// 返回值：
// 已安装：TRUE
// 未安装：FALSE
// ***************/
// void  SetupNet4(LPTSTR Weblink = _T("http://shidaapp.duapp.com/Down/Microsoft.NET.exe"));
//
//
//
//判断系统是否已有此打印机驱动
/***************
参数：
InstallName ： 安装的驱动名称
返回值：
已安装：TRUE
未安装：FALSE
***************/
//BOOL   IsHavePrint(LPCTSTR InstallName);
//获取打印机驱动信息
/***************
参数：
Model ： 驱动型号
返回值：
my_die :自定义结构体
***************/
MY_DRIVER_INFO* GetPrintDriverInfo(LPCTSTR Model);
//卸载打印机驱动
/***************
参数：
my_die :自定义结构体
InstallName ： 安装的驱动名称
返回值：
卸载成功：TRUE
卸载失败：FALSE
***************/
//BOOL  UninstallDriver(LPCTSTR InstallName, MY_DRIVER_INFO *die);

//通过Usb口获取产家信息，设备名称，仿真等
/***************
参数：
PrintName :打印机型号
Provider ：产家名称
Command ： 仿真指令
SelectPro： 筛选条件 例如_T("START")
返回值：
获取成功：TRUE
获取失败：FALSE
***************/
BOOL  GetUSBPrintId(LPTSTR& PrintName, LPTSTR& Provider, LPTSTR& Command, LPTSTR SelectPro = _T(""));

/*
*函数功能：
*
//获取打印机设备ID，并口
*
*函数参数：
*
*		HANDLE hPort 设备句柄
LPTSTR Model 设备型号
LPTSTR Production   产家信息
LPTSTR Simulation  仿真信息
SelectPro： 筛选条件 例如_T("START"),为空则为不筛选
*
返回值：
获取成功：TRUE
获取失败：FALSE
*
*函数返回值：
*
*/
BOOL  GetPrintID_LPT(LPTSTR& Model, LPTSTR& Production, LPTSTR& Simulation, LPTSTR SelectPro = _T(""));

//查找当前连接的打印机的USB端口号
/***************
参数：
VIDPID :VIDPID 筛选条件  例如：_T("vid_xxxx&pid_xxxx")  x:0-9数字
返回值：
未找到端口返回 0：，否则返回端口号码USB001 = 1
***************/
int  FindUSBPortID(LPCTSTR VIDPID);

//枚举所有端口
/***************
返回值：
LPTSTR：所有端口名称，每个端口名称以；隔开
***************/
LPTSTR   EnumerateAllPort();
//复制字体至系统CFont路径下
//BOOL  CopyFont();

//关闭服务
//BOOL  CloseSampleService(LPTSTR Service);
//启动服务
//BOOL  StartSampleService(LPTSTR Service);
//运行卸载程序
//BOOL  ShellUninstallExe(LPTSTR AppPath = _T(""));
//扫描设备管理器
void  ScanForHardwareChanges1();

//BOOL  ChangePrinterHighAttributes(LPCTSTR lpDeviceName, bool bOpen);

//枚举所有打印机驱动
int EnumAllPrintDriver(DRIVER_INFO_6*& die, DWORD& dSize);