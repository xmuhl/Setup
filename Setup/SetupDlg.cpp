﻿// SetupDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Setup.h"
#include "SetupDlg.h"
#include "afxdialogex.h"
#include "UDefault.h"
#include "winspool.h"
#include "DriversInstall.h"
#include "DlgShowAd.h"
#include "DebugStuff.h"
#include <WinUser.h>
#include "DlgFindPrint.h"
#include "LoadLocalResource.h"
#include "DlgMesError.h"
#include <winsvc.h>
#include <setupapi.h>
#include <devguid.h> // 需要链接SetupAPI库
#include <afxmt.h>
#include "GetSysVersion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool g_bFindPrint = false;		// 全局标识符，标识是否找到打印，找到了直接跳到最后一个页面  mod linlc 12.3.23
static bool g_bOpenLink = false;			// 判断是否已点击下载网页链接

// 全局变量 是否中断打印服务
bool g_bStopPrintSpooler = false;

CString g_MesTile = _T("");
CString g_Name = _T("");
CString g_MFG = _T("");
UINT   g_uShowDlg = 1;								// 默认显示对话框
UINT   g_bOnboardDriver = 0;						// 默认非内嵌驱动

typedef BOOL(WINAPI* Wow64RevertWow64FsRedirectionFunc)(PVOID OldValue);
typedef BOOL(WINAPI* Wow64DisableWow64FsRedirectionFunc)(PVOID OldValue);

// 联机状态下卸载驱动子线程函数
UINT DelPrintDriverProcOnLine(LPVOID pParam)
{
	// 将 pParam 转换回 CMFCDemoDlg 类型并进行操作
	CSetupDlg* pThis = (CSetupDlg*)pParam;

	// 调用卸载程序处理函数
	pThis->DeleteSelPrinterDriver(pThis->m_strLinkPriter);

	//Sleep(2000);
	// 安全地关闭提示对话框
	pThis->PostMessage(WM_CLOSE_PROMPT_DLG_ONLINE); // 自定义消息通知主线程关闭对话框

	return 0; // 返回线程退出代码
}

// 脱机状态下卸载驱动子线程函数
UINT DelPrintDriverProcOffLine(LPVOID pParam)
{
	// 将 pParam 转换回 CMFCDemoDlg 类型并进行操作
	CSetupDlg* pThis = (CSetupDlg*)pParam;

	// 调用卸载程序处理函数
	pThis->DeleteSelPrinterDriver(pThis->m_strLinkPriter);

	// 安全地关闭提示对话框
	pThis->PostMessage(WM_CLOSE_PROMPT_DLG_OFFLINE); // 自定义消息通知主线程关闭对话框

	return 0; // 返回线程退出代码
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CSetupDlg 对话框
CSetupDlg::CSetupDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSetupDlg::IDD, pParent)
	, m_iPageCursel(1)
	, m_iPrnServerCounts(0)
	, m_strLinkCommand(_T(""))
	, m_strLinkPriter(_T(""))
	, m_strLinkProduction(_T(""))
	, m_bNewModel(FALSE)
	, m_bStopInstaller(FALSE)
	, m_PreSelItem(0)
	, m_PrnSerialID(0)
	, m_PrnModelID(0)
	, m_strPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hIns = AfxGetInstanceHandle();
	m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
	// 初始化成员变量
	m_ProgressDlg = nullptr;

	//初始化按钮名称
	//m_btnPre.m_strBtnName = _T("上一步(B)");
	//m_btnNext.m_strBtnName = _T("下一步(N)");
	//m_btnCancel.m_strBtnName = _T("取消(E)");
	m_btnClose.m_strBtnName = _T("关闭");
	m_btnMini.m_strBtnName = _T("最小化");
}

// CSetupDlg 对话框的析构函数
CSetupDlg::~CSetupDlg()
{
	// 释放动态分配的 CProgressDlg 对象
	if (m_ProgressDlg)
	{
		m_ProgressDlg->DestroyWindow();
		delete m_ProgressDlg;
		m_ProgressDlg = nullptr;
	}

	// 销毁加载的图标（如果有）
	if (m_hIcon)
	{
		DestroyIcon(m_hIcon);
		m_hIcon = nullptr;
	}

	// 释放加载的库（如果有）
	if (m_hIns)
	{
		FreeLibrary(m_hIns);
		m_hIns = nullptr;
	}

	// 其他需要手动释放的资源
	// 例如，如果您在类中手动分配了其他资源，请在此处释放

	// 判断打印服务是否被中断，如果是，则重新启动打印服务
	if (g_bStopPrintSpooler)
	{
		if (StartPrintSpoolerService())
		{
			g_bStopPrintSpooler = false;
		}
		else {
			CString strInfoMes;
			strInfoMes.Format(_T("重新启动打印服务失败！"));
			MesError(strInfoMes);
		}
	}
}

void CSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SERVER, m_comboServer);
	DDX_Control(pDX, IDC_COMBO_MODEL, m_comboModel);
	DDX_Control(pDX, IDC_BTN_PRE, m_btnPre);
	DDX_Control(pDX, IDC_BTN_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_BTN_CANCLE, m_btnCancel);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
	DDX_Control(pDX, IDC_PrnSerial, m_PrnSerial);
	DDX_Control(pDX, IDC_PrnModel, m_PrnModel);
	DDX_Control(pDX, IDC_LIST1, m_ListPort);
	DDX_Control(pDX, IDC_LOGO, m_Logo);
	DDX_Control(pDX, IDC_STATIC_INFO, m_strInfo);
	DDX_Control(pDX, IDC_SetupInfo, m_SetupInfo);
	DDX_Control(pDX, IDC_BTN_MINIMIZE, m_btnMini);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_btnClose);
}

BEGIN_MESSAGE_MAP(CSetupDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_BTN_PRE, &CSetupDlg::OnBnClickedBtnPre)
	ON_BN_CLICKED(IDC_BTN_NEXT, &CSetupDlg::OnBnClickedBtnNext)
	ON_BN_CLICKED(IDC_BTN_CANCLE, &CSetupDlg::OnBnClickedBtnCancle)
	ON_CBN_SELCHANGE(IDC_COMBO_SERVER, &CSetupDlg::OnCbnSelchangeComboServer)
	ON_CBN_SELCHANGE(IDC_COMBO_MODEL, &CSetupDlg::OnCbnSelchangeComboModel)
	ON_MESSAGE(UM_ENDSTALL, OnEndStall)
	ON_MESSAGE(UM_ENDPROGBAR, OnEndProgressBar)
	ON_WM_TIMER()
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CSetupDlg::OnNMClickList1)
	ON_BN_CLICKED(IDC_BTN_MINIMIZE, &CSetupDlg::OnBnClickedBtnMinimize)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CSetupDlg::OnBnClickedBtnClose)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_CLOSE_PROMPT_DLG_ONLINE, &CSetupDlg::OnClosePromptDialogOnLine)
	ON_MESSAGE(WM_CLOSE_PROMPT_DLG_OFFLINE, &CSetupDlg::OnClosePromptDialogOffLine)
END_MESSAGE_MAP()

//弹出错误信息，自绘窗口,第二个参数为，是直接输出错误信息，还是需要getlasterror
/***************
错误代码：
1：缺少配置文件
2：拒绝拷贝文件
3：安装inf文件失败
4：增加打印机驱动失败
5：写入注册表失败
6：增加打印机失败
7：运行不同位数的程序
9：取消打印任务失败
10：卸载打印机驱动失败
***************/
void MesError(CString strError, BOOL bGetlastError)
{
	CDlgMesError DlgMes;
	if (bGetlastError)
	{
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();
		CString MesBuf;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		if (dw == ERROR_SUCCESS)
		{
			MesBuf.Format(_T("%s succeed:  %lu: %s"), strError, dw, lpMsgBuf);
		}
		else
		{
			MesBuf.Format(_T("%s failed with error %lu: %s"), strError, dw, lpMsgBuf);
		}
		DlgMes.m_strError = MesBuf;
	}
	else
	{
		DlgMes.m_strError = strError;
	}

	DlgMes.DoModal();
}

BOOL StopPrintSpoolerService()
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM == NULL)
	{
		return FALSE;
	}

	SC_HANDLE hService = OpenService(hSCM, _T("Spooler"), SERVICE_STOP | SERVICE_QUERY_STATUS);

	if (hService == NULL)
	{
		CloseServiceHandle(hSCM);
		return FALSE;
	}

	SERVICE_STATUS status;
	ControlService(hService, SERVICE_CONTROL_STOP, &status);

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);
	return TRUE;
}

BOOL StartPrintSpoolerService()
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM == NULL)
	{
		return FALSE;
	}

	SC_HANDLE hService = OpenService(hSCM, _T("Spooler"), SERVICE_START | SERVICE_QUERY_STATUS);

	if (hService == NULL)
	{
		CloseServiceHandle(hSCM);
		return FALSE;
	}

	StartService(hService, 0, NULL);

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);
	return TRUE;
}

// CSetupDlg 消息处理程序

void CSetupDlg::PopulatePortListInView(CListCtrl& m_ListPort)
{
	// 获取系统现有端口列表
	LPTSTR lptPort = EnumerateAllPort();
	if (!lptPort)
	{
		// 如果 EnumerateAllPort 返回空，说明未获取到端口信息
		return;
	}

	int iPrePos = 0, iNextPos;
	CString strfiles = lptPort;
	int iTime = 0;
	while ((iNextPos = strfiles.Find(_T(";"), iPrePos)) != -1)
	{
		CString strTmp = _T("");
		strTmp = strfiles.Mid(iPrePos, iNextPos - iPrePos);
		iPrePos = iNextPos + 1;
		m_ListPort.InsertItem(iTime, strTmp);
		iTime++;
	}

	// 释放枚举端口所分配的内存，避免内存泄漏
	delete[] lptPort;  // 这里假设 EnumerateAllPort 使用 new[] 分配
	lptPort = nullptr;
}

BOOL CSetupDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置图标
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// 对话框居中
	CenterWindow();

	// 设置提示字符串字体
	m_InfoMesFont.CreateFont(0, 0, 0, 0, FW_NORMAL,
		FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("黑体"));
	m_strInfo.SetFont(&m_InfoMesFont);

	CString strCompany((LPCTSTR)IDS_STR_Company);
	m_strInfo.SetWindowTextW(strCompany);

	// 初始化系列、型号列表的字体
	CRect rect;
	m_PrnSerial.GetWindowRect(rect);
	m_ComboxFont.CreateFont(rect.Height() * 7 / 8, 0, 0, 0, FW_NORMAL,
		FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("黑体"));
	m_comboServer.SetFont(&m_ComboxFont);
	m_comboModel.SetFont(&m_ComboxFont);
	m_comboServer.SetItemHeight(-1, rect.Height() + 2);
	m_comboModel.SetItemHeight(-1, rect.Height() + 2);

	// 设置端口列表扩展属性
	DWORD dwStyle = m_ListPort.GetExtendedStyle();
	dwStyle |= LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_ListPort.SetExtendedStyle(dwStyle);
	m_ListPort.GetWindowRect(rect);
	CString strPortList((LPCTSTR)IDS_PortList);
	m_ListPort.InsertColumn(0, strPortList, LVCFMT_LEFT, rect.Width() * 5 / 8);

	// 显示“最小化”和“关闭”按钮
	m_btnMini.ShowWindow(SW_SHOW);
	m_btnClose.ShowWindow(SW_SHOW);

	// 设置静态文本
	GetDlgItem(IDC_STATIC_Serial)->SetWindowText(_T("打印机系列:"));
	GetDlgItem(IDC_STATIC_Model)->SetWindowText(_T("打印机型号:"));

	// 初始化按钮文本
	m_btnPre.SetWindowText(_T("上一步(B)"));
	m_btnNext.SetWindowText(_T("下一步(N)"));
	m_btnCancel.SetWindowText(_T("取消(E)"));

	// 初始化字体
	InitializeFonts();

	// 加载背景图
	LoadImageFromResource(&img, IDB_BackGround, _T("PNG"));

	// 获取配置文件
	if (!m_Driver.GetFile())
	{
		// 配置文件缺失或获取失败
		CString strInfoMes(LPCTSTR(IDS_MissingConfigFile));
		MesError(strInfoMes);
		PostMessage(WM_CLOSE);
		return TRUE;
	}

	// 若配置文件获取成功，加载配置
	AfxBeginThread(AutoApproveProc, NULL, THREAD_PRIORITY_NORMAL);
	m_Driver.GetSetting();
	AfxGetApp()->m_pszAppName = g_Name;

	// 根据厂家设置LOGO显示
	if ((g_MFG == _T("START")) || (g_MFG == _T("实达")))
	{
		m_Logo.Load(m_hIns, IDB_LOGO, _T("PNG"));
	}
	else
	{
		m_Logo.ShowWindow(SW_HIDE);
	}

	// 载入所有可用的系列
	m_iPrnServerCounts = m_Driver.GetAllSeries(m_Seriver);
	for (int i = 0; i < m_iPrnServerCounts; i++)
	{
		m_comboServer.InsertString(i, m_Seriver[i].SeriesName);
	}

	//////////////////////////////////////////////////////////////////////////
	// 新增或修改：判断 g_uShowDlg == 0 时的“静默安装”流程
	//////////////////////////////////////////////////////////////////////////
	if (g_uShowDlg == 0)
	{
		// 1) 创建并调用打印机自动检测对话框（CDlgFindPrint），限定搜索时间
		CDlgFindPrint dlgFind;

		// 【可选】如果想给CDlgFindPrint加一个限定时长，例如 10 秒:
		//        方式之一：在CDlgFindPrint里加定时器或外部线程限制。
		//        这里示例使用外部定时器做演示。
		//        （若已在 CDlgFindPrint 内部编写超时逻辑，可忽略此处。）

		// 用于防止一直卡住不返回，这里仅做示例:
		//   - 设定 10 秒后自动关闭对话框
		//   - 在对话框 OnTimer 或你想要的逻辑里调用 EndDialog(IDCANCEL) 结束
		//   - 具体可视需求实现
		dlgFind.SetTimeout(5); // 设置5秒超时

		// 执行 DoModal 进行自动检测
		int nRetFind = dlgFind.DoModal();

		// 2) 判断自动检测结果
		if (nRetFind == IDOK)
		{
			// 表示成功获取到打印机信息
			m_strLinkPriter = dlgFind.m_strLinkPriter;
			m_strLinkCommand = dlgFind.m_strLinkCommand;
			m_strLinkProduction = dlgFind.m_strLinkProduction;
		}
		else
		{
			// 表示超时/用户取消/未检测到打印机
			// 则使用“配置文件”中的第一系列+第一型号
			if (m_iPrnServerCounts > 0)
			{
				// 安全检查
				m_PrnSerialID = 0;
				int iPrintCount = m_Driver.GetAllPrintModel(m_Seriver[0].SeriesName, m_Printe);
				if (iPrintCount > 0)
				{
					m_PrnModelID = 0; // 第一个型号
					m_strLinkPriter = m_Printe[m_PrnModelID].PrintModel;
				}
			}
		}

		// 3) 根据是否找到打印机信息，自动设置端口
		//    若检测到USB端口则 m_strPort = USBxxx，否则默认为 LPT1:
		BOOL bFindUSB = GetConnectPort();
		// bFindUSB==TRUE时为找到USB端口

		// 4) 无论是否找到打印机，都要检查是否已安装过旧版驱动
		//    需要用 m_strLinkPriter 来做关键字（或自定义方式）调用 FindOemInfs
		//    如果 m_strLinkPriter 为空，可能枚举结果为0，也可视需求追加匹配规则
		CString sSysPath;
		GetWindowsDirectory(sSysPath.GetBuffer(MAX_PATH), MAX_PATH);
		sSysPath.ReleaseBuffer();
		CString searchPath1 = sSysPath + _T("\\INF\\");
		CString targetFile = _T("*.inf");
		FindOemInfs(searchPath1, targetFile, fileList);

		if (fileList.GetCount() > 0)
		{
			// 4.1) 如果检测到已安装旧版驱动
			//      静默模式，直接启动删除驱动的进度对话框(脱机卸载线程)
			m_ProgressDlg = new CProgressDlg();
			m_ProgressDlg->Create(IDD_DLG_PROGRESS, this);
			m_ProgressDlg->ShowWindow(SW_SHOW);

			// 启动卸载子线程 -> 卸载完毕后会自动回调 OnClosePromptDialogOffLine
			AfxBeginThread(DelPrintDriverProcOffLine, this);

			Page2SHowOrHid(false);

			// !!! 此处 return TRUE; 交由卸载完成后再进入安装
			// OnClosePromptDialogOffLine 中会调用 Page5SHowOrHid(TRUE) 开始安装
			return TRUE;
		}
		else
		{
			// 4.2) 没检测到旧版驱动，直接进入Page5执行安装
			Page2SHowOrHid(false);
			Page5SHowOrHid(true);
			return TRUE;
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
	// 如果 g_uShowDlg == 1，执行原有的“有界面”安装引导流程
	//////////////////////////////////////////////////////////////////////////

	// 下面是原始逻辑
		bool bFindConnectUSB = GetConnectPort();
		// 先弹出查找打印机对话框
		{
			CDlgFindPrint dlg;
			if (IDOK == dlg.DoModal())
			{
				PopulatePortListInView(m_ListPort);
				m_strLinkPriter = dlg.m_strLinkPriter;
				m_strLinkCommand = dlg.m_strLinkCommand;
				m_strLinkProduction = dlg.m_strLinkProduction;

				// 枚举系统INF文件，判断是否安装过同型号驱动
				CString sSysPath;
				GetWindowsDirectory(sSysPath.GetBuffer(MAX_PATH), MAX_PATH);
				sSysPath.ReleaseBuffer();
				CString searchPath1 = sSysPath + _T("\\INF\\");
				CString targetFile = _T("*.inf");
				FindOemInfs(searchPath1, targetFile, fileList);

				if (fileList.GetCount() > 0)
				{
					CString strInfoMes;
					strInfoMes.Format(_T("当前系统中已存在该打印机驱动文件，请根据提示关闭打印机电源，安装程序会自动卸载旧版本驱动！\r\n\n点击‘是’已关闭打印机电源！点击‘否’退出安装程序！"));
					int iRet = AfxMessageBox(strInfoMes, MB_YESNO | MB_ICONQUESTION);
					if (iRet == IDNO)
					{
						m_bStopInstaller = TRUE;
						PostMessage(WM_CLOSE);
						return TRUE;
					}
					else if (iRet == IDYES)
					{
						m_ProgressDlg = new CProgressDlg();
						m_ProgressDlg->Create(IDD_DLG_PROGRESS, this);
						m_ProgressDlg->ShowWindow(SW_SHOW);
						AfxBeginThread(DelPrintDriverProcOnLine, this);
					}
				}
				else
				{
					// 已找到打印机直接跳到安装(页4)
					g_bFindPrint = true;
					Page2SHowOrHid(false);
					Page4SHowOrHid(true);
				}
			}
			else
			{
				int iRet = AfxMessageBox(_T("未检测到打印机，或者您使用其它端口连接！\r\n\n请选择是否继续手动安装？"), MB_YESNO | MB_ICONQUESTION);
				if (iRet == IDNO)
				{
					m_bStopInstaller = TRUE;
					PostMessage(WM_CLOSE);
				}
				else if (iRet == IDYES)
				{
					PopulatePortListInView(m_ListPort);
					Page2SHowOrHid(true);
				}
			}
		}
	}

	return TRUE;
}

//BOOL CSetupDlg::OnInitDialog()
//{
//	CDialogEx::OnInitDialog();
//
//	// 将“关于...”菜单项添加到系统菜单中
//	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
//	ASSERT(IDM_ABOUTBOX < 0xF000);
//	CMenu* pSysMenu = GetSystemMenu(FALSE);
//	if (pSysMenu != NULL)
//	{
//		BOOL bNameValid;
//		CString strAboutMenu;
//		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
//		ASSERT(bNameValid);
//		if (!strAboutMenu.IsEmpty())
//		{
//			pSysMenu->AppendMenu(MF_SEPARATOR);
//			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
//		}
//	}
//
//	// 设置图标
//	SetIcon(m_hIcon, TRUE);
//	SetIcon(m_hIcon, FALSE);
//
//	// 对话框居中
//	CenterWindow();
//
//	// 设置提示字符串字体
//	m_InfoMesFont.CreateFont(0, 0, 0, 0, FW_NORMAL,
//		FALSE, FALSE, 0,
//		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
//		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
//		DEFAULT_PITCH | FF_SWISS, _T("黑体"));
//	m_strInfo.SetFont(&m_InfoMesFont);
//
//	CString strCompany((LPCTSTR)IDS_STR_Company);
//	m_strInfo.SetWindowTextW(strCompany);
//
//	// 初始化系列、型号列表的字体
//	CRect rect;
//	m_PrnSerial.GetWindowRect(rect);
//	m_ComboxFont.CreateFont(rect.Height() * 7 / 8, 0, 0, 0, FW_NORMAL,
//		FALSE, FALSE, 0,
//		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
//		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
//		DEFAULT_PITCH | FF_SWISS, _T("黑体"));
//	m_comboServer.SetFont(&m_ComboxFont);
//	m_comboModel.SetFont(&m_ComboxFont);
//
//	m_comboServer.SetItemHeight(-1, rect.Height() + 2);
//	m_comboModel.SetItemHeight(-1, rect.Height() + 2);
//
//	// 设置端口列表扩展属性
//	DWORD dwStyle = m_ListPort.GetExtendedStyle();
//	dwStyle |= LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
//	m_ListPort.SetExtendedStyle(dwStyle);
//	m_ListPort.GetWindowRect(rect);
//	CString strPortList((LPCTSTR)IDS_PortList);
//	m_ListPort.InsertColumn(0, strPortList, LVCFMT_LEFT, rect.Width() * 5 / 8);
//
//	m_btnMini.ShowWindow(SW_SHOW);
//	m_btnClose.ShowWindow(SW_SHOW);
//
//	// 设置静态文本
//	GetDlgItem(IDC_STATIC_Serial)->SetWindowText(_T("打印机系列:"));
//	GetDlgItem(IDC_STATIC_Model)->SetWindowText(_T("打印机型号:"));
//
//	// 初始化按钮
//	m_btnPre.SetWindowText(_T("上一步(B)"));
//	m_btnNext.SetWindowText(_T("下一步(N)"));
//	m_btnCancel.SetWindowText(_T("取消(E)"));
//
//	// 初始化字体
//	InitializeFonts();
//
//	// 加载背景图
//	LoadImageFromResource(&img, IDB_BackGround, _T("PNG"));
//
//	// 获取配置文件
//	if (m_Driver.GetFile())
//	{
//		AfxBeginThread(AutoApproveProc, NULL, THREAD_PRIORITY_NORMAL);
//		m_Driver.GetSetting();
//		AfxGetApp()->m_pszAppName = g_Name;
//
//		if ((g_MFG == _T("START")) || (g_MFG == _T("实达")))
//		{
//			m_Logo.Load(m_hIns, IDB_LOGO, _T("PNG"));
//		}
//		else
//		{
//			m_Logo.ShowWindow(SW_HIDE);
//		}
//
//		// 载入所有可用的系列
//		m_iPrnServerCounts = m_Driver.GetAllSeries(m_Seriver);
//		for (int i = 0; i < m_iPrnServerCounts; i++)
//		{
//			m_comboServer.InsertString(i, m_Seriver[i].SeriesName);
//		}
//
//		// 获取当前USB端口
//		bool bFindConnectUSB = GetConnectPort();
//
//		// 判断是否显示对话框
//		if (g_uShowDlg == 0)
//		{
//			Page2SHowOrHid(false);
//			Page5SHowOrHid(true);
//			return TRUE;
//		}
//
//		CDlgFindPrint dlg;
//		if (IDOK == dlg.DoModal())
//		{
//			PopulatePortListInView(m_ListPort);
//			m_strLinkPriter = dlg.m_strLinkPriter;
//			m_strLinkCommand = dlg.m_strLinkCommand;
//			m_strLinkProduction = dlg.m_strLinkProduction;
//
//			// 枚举系统INF文件，判断是否安装过同型号驱动
//			CString sSysPath;
//			GetWindowsDirectory(sSysPath.GetBuffer(MAX_PATH), MAX_PATH);
//			sSysPath.ReleaseBuffer();
//			CString searchPath1 = sSysPath + _T("\\INF\\");
//			CString targetFile = _T("*.inf");
//			FindOemInfs(searchPath1, targetFile, fileList);
//
//			if (fileList.GetCount() > 0)
//			{
//				CString strInfoMes;
//				strInfoMes.Format(_T("当前系统中已存在该打印机驱动文件，请根据提示关闭打印机电源，安装程序会自动卸载旧版本驱动！\r\n\n点击‘是’已关闭打印机电源！点击‘否’退出安装程序！"));
//				int iRet = AfxMessageBox(strInfoMes, MB_YESNO | MB_ICONQUESTION);
//
//				if (iRet == IDNO)
//				{
//					m_bStopInstaller = TRUE;
//					// 这里改用 PostMessage 避免初始化未完成就销毁导致的潜在问题
//					PostMessage(WM_CLOSE);
//					return TRUE;
//				}
//				else if (iRet == IDYES)
//				{
//					m_ProgressDlg = new CProgressDlg();
//					m_ProgressDlg->Create(IDD_DLG_PROGRESS, this);
//					m_ProgressDlg->ShowWindow(SW_SHOW);
//
//					AfxBeginThread(DelPrintDriverProcOnLine, this);
//				}
//			}
//			else
//			{
//				// 已找到打印机直接跳到安装(页4)
//				g_bFindPrint = true;
//				Page2SHowOrHid(false);
//				Page4SHowOrHid(true);
//			}
//		}
//		else
//		{
//			int iRet = AfxMessageBox(_T("未检测到打印机，或者您使用其它端口连接！\r\n\n请选择是否继续手动安装？"), MB_YESNO | MB_ICONQUESTION);
//
//			if (iRet == IDNO)
//			{
//				m_bStopInstaller = TRUE;
//				PostMessage(WM_CLOSE);
//			}
//			else if (iRet == IDYES)
//			{
//				PopulatePortListInView(m_ListPort);
//				Page2SHowOrHid(true);
//			}
//		}
//	}
//	else
//	{
//		// 配置文件缺失或获取失败
//		CString strInfoMes(LPCTSTR(IDS_MissingConfigFile));
//		MesError(strInfoMes);
//
//		// 此处改用 PostMessage 代替 SendMessage
//		PostMessage(WM_CLOSE);
//		return TRUE;
//	}
//
//	return TRUE;
//}

void CSetupDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
void CSetupDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		CRect rect;
		GetClientRect(&rect);
		img.StretchBlt(dc.m_hDC, rect, SRCCOPY);

		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CSetupDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CSetupDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	//return true;
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CSetupDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnMouseMove(nFlags, point);
}

//鼠标左键点击消息
void CSetupDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	//实现对话框跟随鼠标移动
	::SendMessage(GetSafeHwnd(), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);

	CDialogEx::OnLButtonDown(nFlags, point);
}

// CSetupDlg.cpp中实现字体初始化
void CSetupDlg::InitializeFonts()
{
	CDC* pDC = GetDC();
	if (!pDC) return;

	// 获取系统DPI设置
	int nLogPixelsY = pDC->GetDeviceCaps(LOGPIXELSY);

	// 设置不同的字体大小（以磅为单位）
	const int NORMAL_SIZE = 13;    // 普通文本
	const int TITLE_SIZE = 15;    // 标题文本
	const int LOG_SIZE = 11;       // 日志文本

	// 创建普通字体
	LOGFONT lf = { 0 };
	lf.lfHeight = -MulDiv(NORMAL_SIZE, nLogPixelsY, 72);
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfQuality = CLEARTYPE_QUALITY;
	_tcscpy_s(lf.lfFaceName, _T("Microsoft YaHei UI"));
	m_fontNormal.CreateFontIndirect(&lf);

	// 创建粗体
	lf.lfWeight = FW_BOLD;
	m_fontBold.CreateFontIndirect(&lf);

	// 创建大号标题字体
	lf.lfHeight = -MulDiv(TITLE_SIZE, nLogPixelsY, 72);
	m_fontTitle.CreateFontIndirect(&lf);

	// 创建日志专用字体（较小号）
	lf.lfHeight = -MulDiv(LOG_SIZE, nLogPixelsY, 72);
	lf.lfWeight = FW_NORMAL;
	m_fontLog.CreateFontIndirect(&lf);

	GetDlgItem(IDC_STATIC_Serial)->SetFont(&m_fontTitle);
	GetDlgItem(IDC_STATIC_Model)->SetFont(&m_fontTitle);

	// 设置按钮字体
	m_btnPre.SetFont(&m_fontNormal);
	m_btnNext.SetFont(&m_fontNormal);
	m_btnCancel.SetFont(&m_fontNormal);

	ReleaseDC(pDC);
}

// 显示新的界面，隐藏旧界面
void CSetupDlg::ShowPage(int showiPos, int beforeiPos)
{
	//隐藏旧界面
	switch (beforeiPos)
	{
	case 1:
	{
		Page1SHowOrHid(false);
		break;
	}
	case 2:
	{
		Page2SHowOrHid(false);
		break;
	}
	case 3:
	{
		Page3SHowOrHid(false);
		break;
	}
	case 4:
	{
		Page4SHowOrHid(false);
		break;
	}
	case 5:
	{
		Page5SHowOrHid(false);
		break;
	}
	default:
		break;
	}

	if (!m_bStopInstaller)				// 未中断安装进程
	{
		//显示新界面
		switch (showiPos)
		{
		case 1:
		{
			Page1SHowOrHid(true);
			break;
		}
		case 2:
		{
			//调用函数显示第二个页面
			Page2SHowOrHid(true);
			break;
		}
		case 3:
		{
			//调用函数显示第三个页面
			Page3SHowOrHid(true);
			break;
		}
		case 4:
		{
			//调用函数显示第四个页面
			Page4SHowOrHid(true);
			break;
		}
		case 5:
		{
			//调用函数显示第五个页面
			Page5SHowOrHid(true);
			break;
		}
		default:
			break;
		}
	}
}

// 界面1显示或者隐藏函数
void CSetupDlg::Page1SHowOrHid(bool bMark)
{
	//显示该界面
	if (bMark)
	{
		//标识改变
		m_iPageCursel = 1;
	}
	else
	{
	}
}

// 界面2显示或者隐藏函数
void CSetupDlg::Page2SHowOrHid(bool bMark)
{
	// 根据当前选择的系列和机型对存储对象进行赋值
	OnitPrintList(m_PrnSerialID, m_PrnModelID);

	//显示该界面
	if (bMark)
	{
		//标识改变
		m_iPageCursel = 2;

		// 初始化系列及型号列表
	/*	m_PrnSerial.ShowWindow(SW_SHOW);
		m_PrnModel.ShowWindow(SW_SHOW);*/
		m_comboServer.ShowWindow(SW_SHOW);
		m_comboModel.ShowWindow(SW_SHOW);
		m_Logo.ShowWindow(SW_SHOW);
		m_strInfo.ShowWindow(SW_HIDE);

		// 显示系列和型号静态文本控件
		GetDlgItem(IDC_STATIC_Serial)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_Model)->ShowWindow(SW_SHOW);

		//隐藏上一步按钮
		m_btnPre.ShowWindow(SW_HIDE);

		//显示下一步按钮
		m_btnNext.ShowWindow(SW_SHOW);

		//显示取消按钮
		m_btnCancel.ShowWindow(SW_SHOW);
	}
	else
	{
		//保存要安装的系列及驱动名称
		CString SelServer;
		m_comboServer.GetWindowTextW(SelServer);
		m_StructSetup = GetInstallInfo(m_strLinkPriter, m_strLinkCommand, m_strLinkProduction, SelServer, m_Printe[m_PrnModelID]);  // 获取安装信息
		// 检查返回的安装信息是否为空
		if (m_StructSetup == NULL)
		{
			CString strInfoMes;
			strInfoMes.Format(_T("获取安装信息失败，请检查配置文件！"));
			MesError(strInfoMes);
			SendMessage(WM_CLOSE);
		}
		else
		{
			m_StructSetup->Port = m_strPort;

			//隐藏该界面
			/*m_PrnSerial.ShowWindow(SW_HIDE);
			m_PrnModel.ShowWindow(SW_HIDE);*/
			m_comboServer.ShowWindow(SW_HIDE);
			m_comboModel.ShowWindow(SW_HIDE);
			m_btnPre.ShowWindow(SW_SHOW);
			m_strInfo.ShowWindow(SW_HIDE);

			// 隐藏系列和型号静态文本控件
			GetDlgItem(IDC_STATIC_Serial)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_Model)->ShowWindow(SW_HIDE);
		}
	}
}

// 界面3显示或者隐藏函数
void CSetupDlg::Page3SHowOrHid(bool bMark)
{
	//显示该界面
	if (bMark)
	{
		//标识改变
		m_iPageCursel = 3;

		// 判断当前端口号是否和端口列表中的端口号匹配
		bool bFindPort = false;
		for (int i = 0; i < m_ListPort.GetItemCount(); i++)
		{
			CString strPort = m_ListPort.GetItemText(i, 0);
			if (strPort == m_strPort)
			{
				bFindPort = true;
				m_PreSelItem = i;

				break;
			}
		}

		//显示端口号
		CRect RectMain;
		GetClientRect(RectMain);

		CRect RectButton;
		m_btnCancel.GetWindowRect(RectButton);
		ScreenToClient(RectButton);

		CRect rect;
		m_ListPort.GetWindowRect(rect);
		ScreenToClient(rect);
		rect.top = RectMain.Height() * 5 / 16;
		rect.bottom = RectButton.bottom;
		m_ListPort.MoveWindow(rect, true);
		m_ListPort.ShowWindow(SW_SHOW);

		// 自动滚动到选中条目
		int  nItem = m_ListPort.GetTopIndex();

		CRect rc;
		m_ListPort.GetItemRect(nItem, rc, LVIR_BOUNDS);
		CSize sz(0, (m_PreSelItem - nItem) * rc.Height());
		m_ListPort.Scroll(sz);

		m_ListPort.SetCheck(m_PreSelItem);
		m_ListPort.SetItemState(m_PreSelItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_ListPort.SetFocus();

		m_btnPre.ShowWindow(SW_SHOW);
		m_btnNext.ShowWindow(SW_SHOW);
		m_btnCancel.ShowWindow(SW_SHOW);
	}
	else
	{
		//隐藏界面三的控件
		m_ListPort.ShowWindow(SW_HIDE);

		CString Port = _T("LPT1:");
		// 枚举获取当前选中项
		int iCursel = -1;
		for (int i = 0; i < m_ListPort.GetItemCount(); i++)
		{
			if (m_ListPort.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
			{
				iCursel = i;
				break;
			}
		}

		if (iCursel != -1)
		{
			Port = m_ListPort.GetItemText(iCursel, 0);
		}
		m_StructSetup->Port = Port;
	}
}

// 界面4显示或者隐藏函数
void CSetupDlg::Page4SHowOrHid(bool bMark)
{
	//显示该界面
	if (bMark)
	{
		m_iPageCursel = 4;

		if (g_uShowDlg == 1)   // 显示提示对话框
		{
			// 型号及端口选择完毕，确定准备进行安装
			CString strMesInfo;
			strMesInfo.Format(_T("\n 当前选择安装机型：%s		\r\n 当前设置打印端口：%s		\r\n\n 点击‘是’开始安装，点击‘否’重新选择\n"),
				m_StructSetup->InstallName, m_StructSetup->Port);

			if (IDYES == AfxMessageBox(strMesInfo, MB_YESNO | MB_ICONINFORMATION))
			{
				if (!g_bFindPrint)
				{
					// 检查系统是否已经有指定型号的驱动程序文件存在
					m_strLinkPriter = m_StructSetup->InstallName;
					// 判断当前字符串m_strLinkPriter中是否包含指定的字符串“实达”，如果包含，则把“实达”删除
					if (m_strLinkPriter.Find(_T("实达")) != -1)
					{
						m_strLinkPriter.Replace(_T("实达"), _T(""));
					}

					// 根据找到的打印机型号，枚举系统中的INF文件，如果当前系统中的INF文件已经包含了该打印机驱动的信息，则直接删除旧版驱动后再安装新驱动
					CString sSysPath;
					GetWindowsDirectory(sSysPath.GetBuffer(MAX_PATH), MAX_PATH);
					sSysPath.ReleaseBuffer();

					CString searchPath1 = sSysPath + _T("\\INF\\");
					CString targetFile = _T("*.inf"); // 目标文件名

					FindOemInfs(searchPath1, targetFile, fileList);

					// 通过判断fileList中的个数，判断是否已经安装了该打印机驱动
					if (fileList.GetCount() > 0)
					{
						if (IDYES == MessageBox(_T("发现当前系统已有") + m_StructSetup->InstallName + _T("驱动文件，是否卸载已有驱动文件后，再继续安装？\n\n选择‘否’将继续安装，但不一定能更新成当前版本驱动！"), g_MesTile, MB_YESNO))
						{
							// 初始化提示对话框
							m_ProgressDlg = new CProgressDlg();
							m_ProgressDlg->Create(IDD_DLG_PROGRESS, this);
							m_ProgressDlg->ShowWindow(SW_SHOW);

							// 创建一个工作线程来执行后台操作
							AfxBeginThread(DelPrintDriverProcOffLine, this);
						}
						else
						{
							// 直接跳到安装进度界面
							Page5SHowOrHid(true);
						}
					}
					else
					{
						// 直接跳到安装进度界面
						Page5SHowOrHid(true);
					}
				}
				else
				{
					// 直接跳到安装进度界面
					Page5SHowOrHid(true);
				}
			}
			else
			{
				g_bFindPrint = false;
				// 回到型号选择页面
				Page2SHowOrHid(true);
			}
		}
		else			// 隐藏提示对话框直接进入安装流程
		{
			// 直接跳到安装进度界面
			Page5SHowOrHid(true);
		}
	}
	else
	{
		//隐藏界面四的控件
		m_btnPre.ShowWindow(SW_HIDE);
		m_btnCancel.ShowWindow(SW_HIDE);
		m_btnNext.ShowWindow(SW_HIDE);
	}
}

// 界面5显示或者隐藏函数
void CSetupDlg::Page5SHowOrHid(bool bMark)
{
	//显示该界面
	if (bMark)
	{
		//隐藏按钮控件
		m_btnPre.ShowWindow(SW_HIDE);
		m_btnCancel.ShowWindow(SW_HIDE);
		m_btnNext.ShowWindow(SW_HIDE);
		m_Logo.ShowWindow(SW_HIDE);

		//显示安装进度条
		if (m_SetupInfo.Load(MAKEINTRESOURCE(IDR_GIF2), _T("gif")))
		{
			m_SetupInfo.Draw();
		}

		//标识改变
		m_iPageCursel = 5;

		//设置是否显示百分比
		m_Progress.SetShowPercent(true);

		////设置进度快颜色
		m_Progress.SetBarColor(BarColor);
		SetTimer(IDT_TIMER, 1500, NULL);

		m_Progress.ShowWindow(SW_SHOW);

		m_StructSetup->hWnd = m_hWnd;

		//创建设子进程
		AfxBeginThread(InstallerDriverProc, (LPVOID)m_StructSetup, THREAD_PRIORITY_HIGHEST);
	}
	else
	{
		//隐藏界面五的控件
		m_Progress.ShowWindow(SW_HIDE);
	}
}

//点击上一步按钮
void CSetupDlg::OnBnClickedBtnPre()
{
	// TODO:  在此添加控件通知处理程序代码
	int iPrePage = m_iPageCursel - 1;
	if (iPrePage != 1)   // 不显示第一页内容
	{
		ShowPage(iPrePage, m_iPageCursel);
	}
}

//点击下一步按钮
void CSetupDlg::OnBnClickedBtnNext()
{
	// TODO:  在此添加控件通知处理程序代码
	ShowPage(m_iPageCursel + 1, m_iPageCursel);
}

//点击取消按钮
void CSetupDlg::OnBnClickedBtnCancle()
{
	// TODO:  在此添加控件通知处理程序代码
	SendMessage(WM_CLOSE);
}

//打印机系列改变的消息
void CSetupDlg::OnCbnSelchangeComboServer()
{
	// TODO:  在此添加控件通知处理程序代码

	//获取当前用户选择的系列
	int index = 0;
	index = m_comboServer.GetCurSel();

	//清空所有项目
	m_comboModel.ResetContent();

	int iPrintCount = m_Driver.GetAllPrintModel(m_Seriver[index].SeriesName, m_Printe);
	for (int j = 0; j < iPrintCount; j++)
	{
		m_comboModel.InsertString(j, m_Printe[j].InstallName);
	}

	m_PrnSerialID = index;					// 将当前选择的系列ID赋值给成员变量
	m_comboModel.SetCurSel(0);
	m_PrnModelID = 0;							// 将当前选择的机型ID赋值给成员变量
}

//打印机系列改变的消息
void CSetupDlg::OnCbnSelchangeComboModel()
{
	//// TODO:  在此添加控件通知处理程序代码
	//获取当前用户选择的机型
	int index = 0;
	index = m_comboModel.GetCurSel();

	m_PrnModelID = index;			// 将当前选择的机型ID赋值给成员变量
}

// 初始化打印机系列及型号组合框
void CSetupDlg::OnitPrintList(int iPosSer, int iPosModel)
{
	//m_comboServer.ResetContent();
	m_comboModel.ResetContent();

	int iPrintCount = m_Driver.GetAllPrintModel(m_Seriver[iPosSer].SeriesName, m_Printe);
	for (int j = 0; j < iPrintCount; j++)
	{
		m_comboModel.InsertString(j, m_Printe[j].InstallName);
	}

	m_comboServer.SetCurSel(iPosSer);
	m_comboModel.SetCurSel(iPosModel);

	// 如果当前有打印机连接，则根据连接的打印机型号，自动选择对应的打印机系列和机型
	if (m_strLinkPriter != _T(""))
	{
		int iPosSer = 0, iPosModel = 0;
		if (PrintModelInList(m_strLinkPriter, iPosSer, iPosModel))
		{
			m_comboServer.SetCurSel(iPosSer);
			m_comboModel.SetCurSel(iPosModel);

			// 对应的打印机系列和机型赋值给成员变量
			m_PrnSerialID = iPosSer;
			m_PrnModelID = iPosModel;
		}
	}
}

// 获取当前端口号
BOOL CSetupDlg::GetConnectPort()
{
	BOOL bRet = FALSE;

	int iNum = FindUSBPortID(IDS_VIDPID);
	if (iNum)
	{
		m_strPort.Format(_T("USB%03d"), iNum);
		DebugMesBox(m_strPort);

		bRet = TRUE;
	}
	else
	{
		//没有枚举到Usb有连接打印机,默认勾选并口1
		m_strPort = _T("LPT1:");
	}

	return bRet;
}

// 返回的打印机型号是否在现有列表中
BOOL CSetupDlg::PrintModelInList(CString PrintModel, int& iPosSer, int& iPosModel)
{
	BOOL bFind = FALSE;

	for (int i = 0; i < m_iPrnServerCounts; i++)
	{
		int iPrintCount = m_Driver.GetAllPrintModel(m_Seriver[i].SeriesName, m_Printe);
		for (int j = 0; j < iPrintCount; j++)
		{
			if (PrintModel == m_Printe[j].PrintModel)
			{
				iPosModel = j;
				iPosSer = i;

				bFind = TRUE;
				break;
			}
		}

		if (bFind)
		{
			break;
		}
	}

	return bFind;
}

BOOL CSetupDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO:  在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		OnBnClickedBtnCancle();

		return TRUE;
	}

	if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
	{
		HACCEL hAccel = m_hAccel;
		if (hAccel && ::TranslateAccelerator(m_hWnd, hAccel, pMsg))
		{
			return TRUE;
		}
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (GetDlgItem(IDC_BTN_NEXT) == GetFocus())
		{
			OnBnClickedBtnNext();
			return TRUE;
		}
		else if (GetDlgItem(IDC_BTN_CANCLE) == GetFocus())
		{
			OnBnClickedBtnCancle();
			return TRUE;
		}
		else if (GetDlgItem(IDC_BTN_PRE) == GetFocus())
		{
			OnBnClickedBtnPre();
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

// 停止进度条   add by hl 2017.08.11
LRESULT CSetupDlg::OnEndProgressBar(WPARAM   wParam, LPARAM   lParam)
{
	// 关闭定时器
	KillTimer(IDT_TIMER);

	return 0;
}

//停止滚动条
LRESULT CSetupDlg::OnEndStall(WPARAM   wParam, LPARAM   lParam)
{
	//判断是否安装成功了
	if (lParam)
	{
		m_Progress.SetPos(100);

		// 安装成功，弹出提示对话框，提示指定型号m_strLinkPriter驱动程序成功
		CString strMesInfo;
		strMesInfo.Format(_T("\n%s驱动程序安装成功！\n\n"), m_Printe[m_PrnModelID].InstallName);
		AfxMessageBox(strMesInfo, MB_ICONINFORMATION);
	}
	else
	{
		CString strMesInfo;
		strMesInfo.Format(_T("\n%s驱动程序安装失败！\n\n请联系打印机厂商提供技术支持！"), m_Printe[m_PrnModelID].InstallName);
		AfxMessageBox(strMesInfo, MB_ICONERROR);
	}

	//结束程序
	SendMessage(WM_CLOSE);

	return 0;
}

void CSetupDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
#define DIRECTION_CHANGE_DELAY	20
	//一直是从头走到尾
	int nDirection = +1;
	static int nDirectionChangeDelay = 0;  //标识是走满一段或者刚回到起点，等于0标识正在走，等于20标识在终点

	if (nDirectionChangeDelay > 0)
	{
		nDirectionChangeDelay--;
		m_Progress.RedrawWindow();
	}
	else if (nIDEvent == IDT_TIMER)
	{
		int nPosition = m_Progress.GetPos();

		if ((nDirection == 1) && (nPosition >= 100))
		{
			//在终点
			nDirectionChangeDelay = DIRECTION_CHANGE_DELAY;
			m_Progress.SetPos(0);
		}
		if (nDirectionChangeDelay == 0)
		{
			m_Progress.SetPos(nPosition + nDirection);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

//获取安装信息
SetUp* CSetupDlg::GetInstallInfo(CString LinkPriter, CString LinkCommand, CString  LinkProduction, CString SelServer, PrinterDefInfo info)
{
	SetUp* StrSetup = new SetUp();
	// 检查申请内存是否成功
	if (StrSetup == NULL)
	{
		return NULL;
	}
	StrSetup->Init();

	//安装的名称为厂家加型号
	StrSetup->InstallName = info.InstallName;
	StrSetup->SeriesName = SelServer;
	StrSetup->PrintModel = info.PrintModel;
	if (g_bFindPrint)
	{
		StrSetup->bOnlineSetup = TRUE; //是否在线安装
	}
	else
	{
		StrSetup->bOnlineSetup = FALSE; //是否在线安装
	}
	StrSetup->Point = info.Point;
	StrSetup->infName = info.infName;
	StrSetup->file.DependentFiles = (LPTSTR)(LPCTSTR)info.DependentFiles;
	StrSetup->file.CoverFile = (LPTSTR)(LPCTSTR)info.CoverSYSFile;
	StrSetup->file.MustCopy = (LPTSTR)(LPCTSTR)info.SYSFile;
	StrSetup->GPD = info.DefGPD;

	if (LinkProduction == _T(""))
	{
		StrSetup->MFG = IDS_DefMGF;
	}
	else
	{
		StrSetup->MFG = LinkProduction;
	}

	// 没有返回产品型号基本不能执行在线安装
	if (LinkPriter == _T(""))
	{
		StrSetup->bOnlineSetup = FALSE;
		return StrSetup;
	}

	return StrSetup;
}

void CSetupDlg::OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	DWORD dwPos = GetMessagePos();
	CPoint point(LOWORD(dwPos), HIWORD(dwPos));
	m_ListPort.ScreenToClient(&point);

	LVHITTESTINFO lvinfo;
	lvinfo.pt = point;
	lvinfo.flags = LVHT_ABOVE;

	UINT nFlag;
	int nItem = m_ListPort.HitTest(point, &nFlag);
	//判断是否点在checkbox上
	if (nFlag == LVHT_ONITEMSTATEICON)
	{
		if (nItem != -1)
		{
			if (nItem != m_PreSelItem)
			{
				// 取消之前的勾选项和选中状态
				m_ListPort.SetCheck(m_PreSelItem, false);
				m_ListPort.SetItemState(m_PreSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);

				// 设置当前选中项和选中状态
				m_ListPort.SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

				m_PreSelItem = nItem;
			}
		}
	}
	else
	{
		// 不是点在复选框中，但选中具体某一项
		if (nItem != -1)
		{
			if (nItem != m_PreSelItem)
			{
				// 取消之前的勾选项和选中状态
				m_ListPort.SetCheck(m_PreSelItem, false);
				m_ListPort.SetItemState(m_PreSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);

				// 设置当前选中项和选中状态
				m_ListPort.SetCheck(nItem, true);
				m_PreSelItem = nItem;
			}
		}
	}

	*pResult = 0;
}

// 最小化按钮响应
void CSetupDlg::OnBnClickedBtnMinimize()
{
	// TODO: 在此添加控件通知处理程序代码
	//ShowWindow(SW_MINIMIZE);
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);//最小化
}

// 关闭按钮响应
void CSetupDlg::OnBnClickedBtnClose()
{
	// TODO: 在此添加控件通知处理程序代码
	SendMessage(WM_CLOSE);
}

HBRUSH CSetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何属性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

// 全局变量和同步对象
CRITICAL_SECTION g_taskQueueCS;    // 任务队列临界区
CRITICAL_SECTION g_resultListCS;   // 结果列表临界区
bool g_isInitialized = false;      // 初始化标志

// 搜索任务结构
struct OemInfSearchTask {
	CString path;           // 搜索路径
	CString pattern;        // 搜索模式
	CString searchString;   // 需要在文件中查找的字符串
};

// 全局任务队列和结果列表
std::vector<OemInfSearchTask> g_taskQueue;
std::vector<CString> g_resultList;

// 确保临界区初始化
void EnsureSearchInit() {
	if (!g_isInitialized) {
		InitializeCriticalSection(&g_taskQueueCS);
		InitializeCriticalSection(&g_resultListCS);
		g_isInitialized = true;
	}
}

// 清理资源
void CleanupSearch() {
	if (g_isInitialized) {
		DeleteCriticalSection(&g_taskQueueCS);
		DeleteCriticalSection(&g_resultListCS);
		g_isInitialized = false;
	}
}

// 工作线程函数
DWORD WINAPI OemInfSearchWorker(LPVOID lpParam)
{
	while (true)
	{
		OemInfSearchTask task;
		bool hasTask = false;

		// 获取任务
		EnterCriticalSection(&g_taskQueueCS);
		if (!g_taskQueue.empty()) {
			task = g_taskQueue.back();
			g_taskQueue.pop_back();
			hasTask = true;
		}
		LeaveCriticalSection(&g_taskQueueCS);

		if (!hasTask) break;

		// 执行文件搜索
		CFileFind finder;
		CString strWildcard = task.path + _T("\\") + task.pattern;
		BOOL bWorking = finder.FindFile(strWildcard);

		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			if (finder.IsDots())
				continue;

			if (finder.IsDirectory())
			{
				// 将子目录添加到任务队列
				OemInfSearchTask newTask;
				newTask.path = finder.GetFilePath();
				newTask.pattern = task.pattern;
				newTask.searchString = task.searchString;

				EnterCriticalSection(&g_taskQueueCS);
				g_taskQueue.push_back(newTask);
				LeaveCriticalSection(&g_taskQueueCS);
			}
			else
			{
				CString strFilePath = finder.GetFilePath();
				BOOL bIsSelFile = FALSE;
				CStdioFile file;

				if (file.Open(strFilePath, CFile::modeRead | CFile::typeBinary))
				{
					UINT nFileSize = (UINT)file.GetLength();
					LPSTR pszFileContent = new CHAR[nFileSize + 1];
					file.Read(pszFileContent, nFileSize);
					pszFileContent[nFileSize] = '\0';

					int nWideChars = MultiByteToWideChar(CP_ACP, 0, pszFileContent, -1, NULL, 0);
					LPWSTR pwszFileContent = new WCHAR[nWideChars];
					MultiByteToWideChar(CP_ACP, 0, pszFileContent, -1, pwszFileContent, nWideChars);

					CStringW strFileContentW(pwszFileContent);
					int nPos = -1;
					int startPos = 0, endPos = 0;

					while (endPos != -1)
					{
						endPos = strFileContentW.Find('\n', startPos);
						if (endPos != -1)
						{
							CStringW strLineW = strFileContentW.Mid(startPos, endPos - startPos);
							startPos = endPos + 1;

							if (strLineW.GetLength() > 4)
							{
								strLineW = strLineW.Right(strLineW.GetLength() - 4);
								CString strReverseLine(strLineW);
								strReverseLine.MakeReverse();

								CString strSearchString = task.searchString;
								strSearchString.MakeReverse();

								nPos = strReverseLine.Find(strSearchString);
								if (nPos != -1)
								{
									nPos = strLineW.GetLength() - (nPos + strSearchString.GetLength());
									bIsSelFile = TRUE;
									break;
								}
							}
						}
						else break;
					}

					delete[] pszFileContent;
					delete[] pwszFileContent;
					file.Close();
				}

				if (bIsSelFile)
				{
					// 将找到的文件添加到结果列表
					CString strFileName = finder.GetFileName();
					EnterCriticalSection(&g_resultListCS);
					g_resultList.push_back(strFileName);
					LeaveCriticalSection(&g_resultListCS);
				}
			}
		}
		finder.Close();
	}

	return 0;
}

// 改进后的主查找函数
void CSetupDlg::FindOemInfs(const CString& strPath, const CString& strPattern,
	CList<CString, CString&>& fileList)
{
	try {
		// 初始化同步对象
		EnsureSearchInit();

		// 清空全局数据
		EnterCriticalSection(&g_taskQueueCS);
		g_taskQueue.clear();
		LeaveCriticalSection(&g_taskQueueCS);

		EnterCriticalSection(&g_resultListCS);
		g_resultList.clear();
		LeaveCriticalSection(&g_resultListCS);

		// 创建初始任务
		OemInfSearchTask initialTask;
		initialTask.path = strPath;
		initialTask.pattern = strPattern;
		initialTask.searchString = m_strLinkPriter;

		EnterCriticalSection(&g_taskQueueCS);
		g_taskQueue.push_back(initialTask);
		LeaveCriticalSection(&g_taskQueueCS);

		// 创建工作线程
		const int threadCount = 4;  // 使用4个工作线程
		std::vector<HANDLE> threads;

		for (int i = 0; i < threadCount; ++i)
		{
			HANDLE hThread = CreateThread(NULL, 0, OemInfSearchWorker, NULL, 0, NULL);
			if (hThread)
			{
				threads.push_back(hThread);
			}
		}

		// 等待所有线程完成
		if (!threads.empty())
		{
			WaitForMultipleObjects((DWORD)threads.size(), threads.data(), TRUE, INFINITE);
		}

		// 将结果转移到输出列表 - 修正类型匹配问题
		EnterCriticalSection(&g_resultListCS);
		for (const auto& fileName : g_resultList)
		{
			CString tempStr = fileName;  // 创建一个新的CString对象
			fileList.AddTail(tempStr);   // 添加这个对象的引用
		}
		LeaveCriticalSection(&g_resultListCS);

		// 清理线程句柄
		for (auto hThread : threads)
		{
			CloseHandle(hThread);
		}
	}
	catch (...) {
		// 确保在发生异常时清理资源
		CleanupSearch();
		throw;
	}
}

//void CSetupDlg::FindOemInfs(const CString& strPath, const CString& strPattern, CList<CString, CString&>& fileList)
//{
//	CFileFind finder;
//	CString strWildcard = strPath + "\\" + strPattern;
//
//	BOOL bWorking = finder.FindFile(strWildcard);
//
//	while (bWorking)
//	{
//		bWorking = finder.FindNextFile();
//
//		if (finder.IsDots())
//			continue;
//
//		if (finder.IsDirectory())
//		{
//			CString str = finder.GetFilePath();
//			FindOemInfs(str, strPattern, fileList);
//		}
//		else
//		{
//			CString strFilePath = finder.GetFilePath();
//
//			// 判断当前文件是否包含指定字符串内容
//			BOOL bIsSelFile = FALSE;
//			CStdioFile file;
//			DWORD dwErr = 0;
//			if (file.Open(strFilePath, CFile::modeRead | CFile::typeBinary))
//			{
//				// Determine the file size
//				UINT nFileSize = (UINT)file.GetLength();
//
//				// Allocate a buffer to hold the file contents
//				LPSTR pszFileContent = new CHAR[nFileSize + 1];
//
//				// Read the file into the buffer
//				file.Read(pszFileContent, nFileSize);
//
//				// Add a null terminator to the buffer
//				pszFileContent[nFileSize] = '\0';
//
//				// Convert the ANSI buffer to a Unicode string
//				int nWideChars = MultiByteToWideChar(CP_ACP, 0, pszFileContent, -1, NULL, 0);
//				LPWSTR pwszFileContent = new WCHAR[nWideChars];
//				MultiByteToWideChar(CP_ACP, 0, pszFileContent, -1, pwszFileContent, nWideChars);
//
//				// Search for the specified string
//				//int nPos = CString(pwszFileContent).Find(m_strLinkPriter.GetBuffer());
//				//
//				// Use the in-memory, wide-char representation of the file for line-by-line processing
//				CStringW strFileContentW(pwszFileContent);
//				int nPos = -1;
//
//				int startPos = 0, endPos = 0;
//				while (endPos != -1)
//				{
//					endPos = strFileContentW.Find('\n', startPos);
//					if (endPos != -1)
//					{
//						CStringW strLineW = strFileContentW.Mid(startPos, endPos - startPos);
//						startPos = endPos + 1;  // 更新 startPos 以跳过找到的 '\n'
//
//						// 移除最后4个字符
//						strLineW = strLineW.Right(strLineW.GetLength() - 4);
//
//						// 反转字符串以便从末尾开始查找
//						CString strReverseLine(strLineW);
//						strReverseLine.MakeReverse();
//
//						// 同样反转搜索字符串
//						CString strSearchString = m_strLinkPriter.GetBuffer();
//						strSearchString.MakeReverse();
//
//						// 执行查找
//						nPos = strReverseLine.Find(strSearchString);
//
//						// 如果找到了，修正索引值以反映原始字符串中的位置
//						if (nPos != -1)
//						{
//							nPos = strLineW.GetLength() - (nPos + strSearchString.GetLength());
//							break;
//						}
//					}
//					else
//					{
//						// 没有找到 '\n'，即已到达文件末尾
//						break;
//					}
//				}
//
//				if (nPos != -1)
//				{
//					// Found the string
//					bIsSelFile = TRUE;
//				}
//
//				// Clean up
//				delete[] pszFileContent;
//				pszFileContent = nullptr;
//				delete[] pwszFileContent;
//				pwszFileContent = nullptr;
//
//				file.Close();
//			}
//			else
//			{
//				dwErr = GetLastError();
//			}
//
//			if (bIsSelFile)
//			{
//				CString strFileName = finder.GetFileName();
//				fileList.AddTail(strFileName);
//			}
//		}
//	}
//
//	finder.Close();
//}

void CSetupDlg::UninstallDriverInf(const CString& driverFile)
{
	// 获取系统临时文件夹路径
	TCHAR szTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, szTempDir);

	// 创建临时批处理文件的完整路径
	CString strTempFile;
	strTempFile.Format(_T("%stempBatchFile.bat"), szTempDir);

	// 添加 ANSI 编码设置
#pragma execution_character_set("utf-8")

// 创建并写入临时批处理文件
	CStdioFile file;
	if (file.Open(strTempFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
	{
		CString strBatchContent;
		strBatchContent = _T("@echo off\r\n");
		strBatchContent += _T("set DRIVER_NAME=%1\r\n");
		strBatchContent += _T("echo Uninstalling driver %DRIVER_NAME%...\r\n");
		strBatchContent += _T("pnputil -f -d %DRIVER_NAME%\r\n");
		strBatchContent += _T("echo Driver %DRIVER_NAME% uninstalled.\r\n");

		file.WriteString(strBatchContent);
		file.Close();
	}
	else
	{
		AfxMessageBox(_T("Could not create batch file."), MB_ICONERROR);

		// 获取错误信息并输出日志
		GetErrAndLog(_T("Could not create batch file."));

		return;
	}

	// 执行临时批处理文件
	HMODULE hModule = LoadLibrary(TEXT("Kernel32.dll"));
	if (hModule == NULL)
	{
		GetErrAndLog(_T("Failed to load Kernel32.dll"));
		return;
	}

	Wow64RevertWow64FsRedirectionFunc Wow64RevertWow64FsRedirectionPtr =
		(Wow64RevertWow64FsRedirectionFunc)GetProcAddress(hModule, "Wow64RevertWow64FsRedirection");

	Wow64DisableWow64FsRedirectionFunc Wow64DisableWow64FsRedirectionPtr =
		(Wow64DisableWow64FsRedirectionFunc)GetProcAddress(hModule, "Wow64DisableWow64FsRedirection");

	if (Wow64RevertWow64FsRedirectionPtr == NULL)
	{
		GetErrAndLog(_T("Wow64RevertWow64FsRedirection function not found."));
	}
	else
	{
		PVOID OldValue;
		// 判断当前操作系统是32位系统还是64位系统，根据当前系统架构选择是否禁用文件系统重定向
		if (Is64OS())
		{
			if (Wow64DisableWow64FsRedirectionPtr(&OldValue))
			{
				ShellExecute(NULL, _T("runas"), strTempFile, driverFile, NULL, SW_HIDE);

				Wow64RevertWow64FsRedirectionPtr(OldValue);

				// 等待批处理文件执行完毕
				Sleep(1000);

				// 输出日志
				CString strMes;
				strMes.Format(_T("在x64系统上运行脚本删除驱动文件%s完成！"), driverFile);
				DebugLogOutput(strMes);
				//DebugLogOutput(_T("在x64系统上运行删除驱动脚本完成！"));
			}
			else
			{
				// 获取错误信息并输出日志
				GetErrAndLog(_T("Wow64DisableWow64FsRedirection failed!"));
			}
		}
		else
		{
			ShellExecute(NULL, _T("runas"), strTempFile, driverFile, NULL, SW_HIDE);

			// 等待批处理文件执行完毕
			Sleep(1000);

			// 输出日志
			CString strMes;
			strMes.Format(_T("在x86系统上运行脚本删除驱动文件%s完成！"), driverFile);
			DebugLogOutput(strMes);
		}
	}

	FreeLibrary(hModule);

	// 等待批处理文件执行完毕
	Sleep(1000);

	// 删除临时批处理文件
	if (DeleteFile(strTempFile) == 0)
	{
		//AfxMessageBox(_T("Could not delete batch file."), MB_ICONERROR);
		// 获取错误信息并输出日志
		GetErrAndLog(_T("Could not delete batch file."));
	}
	else
	{
		// 输出日志
		DebugLogOutput(_T("删除临时批处理文件完成！"));
	}
}

BOOL CSetupDlg::DeleteSelPrinterDriver(CString strPrnModel)
{
	// 对传入的打印机驱动名称进行处理，加入前缀“实达”
	//strPrnModel = _T("实达") + strPrnModel;
	strPrnModel = g_MFG + strPrnModel;

	// 设置获取打印机驱动句柄的权限
	PRINTER_DEFAULTS defaults;
	defaults.pDatatype = NULL;
	defaults.pDevMode = NULL;
	defaults.DesiredAccess = PRINTER_ALL_ACCESS;

	HANDLE hPrinter = NULL;
	// 根据上述设置获取指定名称的打印机驱动句柄
	if (!OpenPrinter((LPWSTR)strPrnModel.GetBuffer(), &hPrinter, &defaults))
	{
		// 调用GetErrAndLog函数，输出日志
		GetErrAndLog(_T("OpenPrinter failed!"));
	}

	// 当传入的句柄不为NULL时，DeletePrinter函数将删除指定的打印机驱动(设备管理器界面上的图标）
	if (hPrinter != NULL)
	{
		// 调用CancelPendingPrintJobs函数取消打印任务
		CancelPendingPrintJobs(hPrinter);

		// 尝试删除打印机
		BOOL bRet = DeletePrinter(hPrinter);
		if (!bRet)
		{
			// 调用GetErrAndLog函数，输出日志
			GetErrAndLog(_T("DeletePrinter failed!"));
		}

		// 关闭打印机句柄
		ClosePrinter(hPrinter);
	}

	// 根据fileList中的文件名,逐一删除指定的inf文件
	POSITION pos = fileList.GetHeadPosition();
	while (pos != NULL)
	{
		CString strFile = fileList.GetNext(pos);

		UninstallDriverInf(strFile);
	}

	// 关闭打印服务
	g_bStopPrintSpooler = StopPrintSpoolerService();

	if (g_bStopPrintSpooler)
	{
		// 等待1秒
		Sleep(1500);

		// 再次启动打印服务
		if (StartPrintSpoolerService())
		{
			g_bStopPrintSpooler = FALSE; // 重置标志

			// 等待1秒
			Sleep(1500);

			// 删除指定的打印机驱动文件
			if (!DeletePrinterDriverEx(NULL, NULL, strPrnModel.GetBuffer(), DPD_DELETE_UNUSED_FILES, 0))
			{
				// 调用GetErrAndLog函数，输出日志
				GetErrAndLog(_T("DeletePrinterDriverEx failed!"));

				return FALSE;
			}
		}
		else {
			// 调用GetErrAndLog函数，输出日志
			GetErrAndLog(_T("StartPrintSpoolerService failed!"));

			return FALSE;
		}
	}
	else {
		// 调用GetErrAndLog函数，输出日志
		GetErrAndLog(_T("StopPrintSpoolerService failed!"));

		return FALSE;
	}

	return TRUE;
}

BOOL CSetupDlg::CancelPendingPrintJobs(HANDLE hPrinter)
{
	// 取消未完成的打印任务
	CStringArray arrJobsToCancel;
	CArray<DWORD, DWORD> arrJobIds; // 用于存储作业ID
	DWORD dwBytesNeeded, dwJobs;
	DWORD dwDeletedJobs = 0;

	// 第一次调用 EnumJobs 来获取所需的缓冲区大小
	if (!EnumJobs(hPrinter, 0, -1, 1, NULL, 0, &dwBytesNeeded, &dwJobs)) {
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			JOB_INFO_1* pJobInfo = (JOB_INFO_1*)malloc(dwBytesNeeded);
			if (pJobInfo == NULL) {
				GetErrAndLog(_T("Memory allocation failed!"));
				return FALSE;
			}

			// 第二次调用 EnumJobs 来获取作业信息
			if (EnumJobs(hPrinter, 0, -1, 1, (LPBYTE)pJobInfo, dwBytesNeeded, &dwBytesNeeded, &dwJobs)) {
				for (DWORD i = 0; i < dwJobs; i++) {
					arrJobsToCancel.Add(pJobInfo[i].pDocument);
					arrJobIds.Add(pJobInfo[i].JobId); // 存储作业ID
				}
			}
			free(pJobInfo);
		}
	}

	// 取消打印任务
	for (int i = 0; i < arrJobsToCancel.GetSize(); i++) {
		CString strJobName = arrJobsToCancel.GetAt(i);
		DWORD pcbNeeded;

		// 使用从 EnumJobs 中获取的作业ID
		DWORD realJobID = arrJobIds.GetAt(i);

		// 第一次调用 GetJob 来获取缓冲区大小
		GetJob(hPrinter, realJobID, 2, NULL, 0, &pcbNeeded);

		if (pcbNeeded > 0) {
			BYTE* pBuffer = new BYTE[pcbNeeded];

			// 第二次调用 GetJob 来获取作业信息
			if (GetJob(hPrinter, realJobID, 2, pBuffer, pcbNeeded, &pcbNeeded)) {
				JOB_INFO_2* pJobInfo = reinterpret_cast<JOB_INFO_2*>(pBuffer);

				if (pJobInfo->pDocument && strJobName.CompareNoCase(pJobInfo->pDocument) == 0) {
					if (!SetJob(hPrinter, pJobInfo->JobId, 0, NULL, JOB_CONTROL_CANCEL)) {
						CString errorMsg;
						errorMsg.Format(_T("Failed to cancel job %d"), pJobInfo->JobId);
						GetErrAndLog(errorMsg);
					}
					else {
						dwDeletedJobs++;
					}
				}
			}
			else {
				GetErrAndLog(_T("Failed to get job information"));
			}
			delete[] pBuffer;
		}
		else {
			GetErrAndLog(_T("Failed to get buffer size for job information"));
		}
	}

	if (dwDeletedJobs > 0) {
		CString strMessage;
		strMessage.Format(_T("Cancelled %d print job(s)"), dwDeletedJobs);
		DebugLogOutput(strMessage);
	}
	else {
		DebugLogOutput(_T("No print jobs found to cancel"));
	}

	return TRUE;
}

// 实现 OnClosePromptDialog 函数
LRESULT CSetupDlg::OnClosePromptDialogOnLine(WPARAM wParam, LPARAM lParam)
{
	m_ProgressDlg->DestroyWindow();
	delete m_ProgressDlg;
	m_ProgressDlg = nullptr;

	// 提示删除指定打印机驱动成功
	CString strMessage;
	strMessage.Format(_T("删除%s打印机驱动完成！\r\n\n如发现仍有打印机图标滞留在“设备和打印机”页面上，不必理会，重新开启打印机后会自动消失。如果介意，可以手动点击“删除设备”删除滞留的打印机图标。"), m_strLinkPriter);
	AfxMessageBox(strMessage, MB_OK);

	// 弹出提示信息，提示用户重新打开打印机电源继续安装驱动
	CString strInfoMes1;
	strInfoMes1.Format(_T("请重新打开打印机电源，安装程序会自动安装新版本驱动！\r\n\n点击‘是’已打开打印机电源！点击‘否’退出安装程序！"));
	int iRet1 = AfxMessageBox(strInfoMes1, MB_YESNO | MB_ICONQUESTION);
	// 退出安装
	if (iRet1 == IDNO)
	{
		m_bStopInstaller = TRUE;				// 中止安装进程
		SendMessage(WM_CLOSE);

		return 0;
	}
	else if (iRet1 == IDYES)
	{
		// 重新枚举当前联机设备
		CDlgFindPrint dlg;
		if (IDOK == dlg.DoModal())
		{
			// 获取当前联机的打印机端口号
			GetConnectPort();

			m_strLinkPriter = dlg.m_strLinkPriter;
			m_strLinkCommand = dlg.m_strLinkCommand;
			m_strLinkProduction = dlg.m_strLinkProduction;

			//获取设备ID成功
			g_bFindPrint = true;   //全局标识符，标识是否找到打印，找到了直接跳到最后一个页面  mod linlc 12.3.23

			Page2SHowOrHid(false);		  // 隐藏当前页面
			m_btnPre.ShowWindow(SW_HIDE);
			Page4SHowOrHid(true);		  // 直接进入安装界面
		}
		else
		{
			int iRet = AfxMessageBox(_T("未检测到打印机，或者您使用其它端口连接！\r\n\n请选择是否继续手动安装？"), MB_YESNO | MB_ICONQUESTION);
			// 退出安装
			if (iRet == IDNO)
			{
				m_bStopInstaller = TRUE;				// 中止安装进程
				SendMessage(WM_CLOSE);
			}
			else if (iRet == IDYES)
			{
				//直接显示第二页
				Page2SHowOrHid(true);
			}
		}
	}
	return 0; // 返回值不会被使用
}

LRESULT CSetupDlg::OnClosePromptDialogOffLine(WPARAM wParam, LPARAM lParam)
{
	m_ProgressDlg->DestroyWindow();
	delete m_ProgressDlg;
	m_ProgressDlg = nullptr;

	if (g_uShowDlg == 1)   // 显示提示对话框
	{
		// 提示删除指定打印机驱动成功
		CString strMessage;
		strMessage.Format(_T("删除%s打印机驱动完成！\r\n\n如发现仍有打印机图标滞留在“设备和打印机”页面上，不必理会，重新开启打印机后会自动消失。如果介意，可以手动点击“删除设备”删除滞留的打印机图标。"), m_strLinkPriter);
		AfxMessageBox(strMessage, MB_OK);
	}

	// 直接进入安装界面
	Page5SHowOrHid(TRUE);

	return 0; // 返回值不会被使用
}