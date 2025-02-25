#pragma once

//#include "..\InstallDll\ExportHead.h"
#include "ExportHead.h"

#define WhiteColor  RGB(255,255,255)
#define InstallFrameCorlor   RGB(128,128,128)			//立即安装边框颜色
#define DlgFramePenSize 2											//主对话框边框粗细
#define DlgFrameColor RGB(51,51,51)						//主对话框边框颜色

//第五页面各个控件
#define MarqueeOptions  30								//进度条每小块多少像素
#define IDT_TIMER 129											//进度条定时器编号
#define BMP_PAGE5TIP1Rect  50,195,125,25	//正在下载提示信息
#define BarColor RGB(0,78,161)							//进度条进度块颜色

#define SerivesMaxNum	512		 //打印机系列最大数量
#define PrintMaxNum		512		//打印机型号最大数量

#define DELAY_TIME			1200   // 结束对话框停驻时长

#define  ENDIMAGE_WIDTH		632
#define  ENDIMAGE_HEIGHT		356

typedef struct
{
	CString SeriesName;
}Serives;

typedef struct
{
	CString PrintModel;
	CString DefGPD;
	CString Point;						//多少针标识
	CString infName;
	CString InstallerSort;			// 按类型区分安装文件
	CString DependentFiles;	//依赖文件
	CString CoverSYSFile;			//需要覆盖的文件
	CString SYSFile;					//拷贝文件
	CString InstallName;			//安装显示的名称
}PrinterDefInfo;

struct SetUp
{
	HWND hWnd;                    // 第四个页面的窗口
	CString SeriesName;           // 打印机系列
	CString PrintModel;           // 打印机型号
	CString MFG;                  // 设备ID中的MFG
	CString InstallName;          // 安装显示的名称
	CString GPD;                  // 安装的GPD名称
	CString Point;                // 安装的针数及文件
	CString Port;                 // 用户选择的打印机端口
	BOOL bOnlineSetup;            // 是否在线安装
	BOOL bNewModel;               // 是否为新增机型
	InstallFile file;             // 安装文件信息
	CString infName;              // INF文件名称

	// 添加一个初始化函数
	void Init() {
		hWnd = NULL;
		bOnlineSetup = FALSE;
		bNewModel = FALSE;
		file.DependentFiles = NULL;
		file.CoverFile = NULL;
		file.MustCopy = NULL;
		// CString 成员不需要特别初始化，它们的构造函数会处理
	}
};

typedef struct
{
	CString strLinkPriter;				//打印机型号
	CString strLinkCommand;		//连接的仿真
	CString strProduction;			//设备ID中的MFG
	CString Port;								//用户选择的打印机端口
}PrintId;

#define BUFSIZE 512

//inf名称
#define IDS_SelectPro  _T("START")							//筛选产家条件，获取ID时候使用
#define IDS_DefMGF  _T("START ")								//默认MFG
#define IDS_Setting  _T("Setting")
#define IDS_MesTile   _T("MesTile")
#define IDS_APPNAME _T("Name")
#define IDS_MFG _T("MFG")							// 配置文件中的MFG字段名
#define IDS_SHOWDLG	 _T("ShowDlg")					// ini文件中是否显示对话框的字段名
#define IDS_OnboardDriver  _T("OnboardDriver")		// ini文件中是否是内置驱动的字段名
#define IDS_VIDPID _T("vid_1bc3&pid_0003")			//实达vidpid
//#define IDS_BootFolderName _T("实达打印机")			//copy工具集及手册时根目录文件名称
#define IDS_DependentFiles _T("DependentFiles")
#define ID_SSYSFile _T("SYSFile")
#define IDS_CoverSYSFile _T("CoverSYSFile")
#define IDS_GPD _T("GPD")
#define  IDS_DriverFile _T("UNIDRV.DLL")
#define  IDS_DataType1 _T("RAW")
#define IDS_Manufacturer _T("START")
#define IDS_PrintProcessor _T("winprint")
#define IDS_CopyName  _T(" (副本 %d)")
#define IDS_ConfigFile _T("unidrvui.dll")
#define IDS_HelpFile _T("unidrv.hlp")
#define IDS_RESFILE _T("UNIRES.DLL")