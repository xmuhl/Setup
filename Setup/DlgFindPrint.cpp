// DlgFindPrint.cpp : 实现文件
//

#include "stdafx.h"
#include "Setup.h"
#include "DlgFindPrint.h"
#include "afxdialogex.h"
#include "UDefault.h"
#include "SetupDlg.h"
#include "IOControl.h"
#include "DebugStuff.h"

BOOL g_bKeepFind = TRUE; //是否继续寻找打印机

// CDlgFindPrint 对话框

extern CString g_MesTile;
extern CString g_Name;
extern UINT g_bOnboardDriver;
// 添加拓展全局变量用于标识是否显示对话框
extern UINT g_uShowDlg;

IMPLEMENT_DYNAMIC(CDlgFindPrint, CDialogEx)

CDlgFindPrint::CDlgFindPrint(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgFindPrint::IDD, pParent)
	, m_strLinkPriter(_T(""))
	, m_strLinkCommand(_T(""))
	, m_strLinkProduction(_T(""))
{
	m_hIns = AfxGetInstanceHandle();
}

CDlgFindPrint::~CDlgFindPrint()
{
	// 释放资源
	m_Pic.UnLoad();
}

void CDlgFindPrint::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PIC_WAIT, m_Pic);
	DDX_Control(pDX, IDC_STATIC1, m_strConnectMes);
	DDX_Control(pDX, IDC_TOP__BANNER2, m_TopBanner);
}

BEGIN_MESSAGE_MAP(CDlgFindPrint, CDialogEx)
	ON_MESSAGE(UM_FINDPRINT, OnFindPrint)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, &CDlgFindPrint::OnBnClickedCancel)
	ON_BN_CLICKED(ID_btnCustomInstall, &CDlgFindPrint::OnBnClickedbtncustominstall)
END_MESSAGE_MAP()

// CDlgFindPrint 消息处理程序

BOOL CDlgFindPrint::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(g_Name);

	// TODO:  在此添加额外的初始化
	if (m_Pic.Load(MAKEINTRESOURCE(IDR_GIF1), _T("gif")))
	{
		m_Pic.Draw();
	}

	CString strInfoMes(LPCTSTR(IDS_ConnectMes));
	m_strConnectMes.SetWindowText(strInfoMes);

	//创建设子进程
	AfxBeginThread(ThreadFindGetID, (LPVOID)m_hWnd, THREAD_PRIORITY_NORMAL);

	// 如果设置了超时时间,则启动定时器
	if (m_nTimeoutSeconds > 0)
	{
		SetTimer(1, m_nTimeoutSeconds * 1000, NULL);
	}

	// 如果不启用对话框，则“用户自定义”按钮不可用
	if (g_uShowDlg == 0)
	{
		GetDlgItem(ID_btnCustomInstall)->EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}
LRESULT  CDlgFindPrint::OnFindPrint(WPARAM   wParam, LPARAM   lParam)
{
	if (wParam)
	{
		EndDialog(IDCANCEL);
	}
	else
	{
		PrintId* ID = (PrintId*)lParam;
		// 存放当前连接的打印机型号
		m_strLinkPriter = ID->strLinkPriter;
		// 标识连接的机器的仿真
		m_strLinkCommand = ID->strLinkCommand;
		// 获取设备ID的产家名称s
		m_strLinkProduction = ID->strProduction;

		EndDialog(IDOK);
	}
	return  0;
}

// 获取设备ID
BOOL GetPrintID(CString& strLinkPriter, CString& strLinkCommand, CString& strProduction, LPTSTR SelectPro)
{
	BOOL bRet = FALSE;

	//int i = 1;		// 减少循环查找次数，缩短等待时长
	do
	{
		//从USB端口获取设备ID
		TCHAR* Model = new TCHAR[MAX_PATH];
		TCHAR* Production = new TCHAR[MAX_PATH];
		TCHAR* Simulation = new TCHAR[MAX_PATH];
		ZeroMemory(Model, MAX_PATH);
		ZeroMemory(Production, MAX_PATH);
		ZeroMemory(Simulation, MAX_PATH);

		if (GetUSBPrintId(Model, Production, Simulation, SelectPro))
		{
			if (_tcslen(Model))
			{
				strLinkPriter = Model;
				strLinkCommand = Simulation;
				strProduction = Production;

				bRet = TRUE;
				delete[] Model;
				Model = NULL;
				delete[] Production;
				Production = NULL;
				delete[] Simulation;
				Simulation = NULL;

				break;
			}
		}     //从并口获取设备ID
		else  if (GetPrintID_LPT(Model, Production, Simulation, SelectPro))
		{
			if (_tcslen(Model))
			{
				strLinkPriter = Model;
				strLinkCommand = Simulation;
				strProduction = Production;

				bRet = TRUE;
				delete[] Model;
				Model = NULL;
				delete[] Production;
				Production = NULL;
				delete[] Simulation;
				Simulation = NULL;

				break;
			}
		}

		ScanForHardwareChanges1();
		Sleep(1000);

		delete[] Model;
		Model = NULL;
		delete[] Production;
		Production = NULL;
		delete[] Simulation;
		Simulation = NULL;
	} while (g_bKeepFind); 		 // 改为一直循环等待
	//} while (i-- && g_bKeepFind);

	return bRet;
}

BOOL SendInitializationDataToUSBPort(HWND hDlg)
{
	// 获取当前连接的USB端口句柄
	CString strModel;
	CString PIDVID = _T("vid_1bc3&pid_0003");
	HANDLE hUSBPort = OpenUSBPort(strModel, PIDVID);
	if (hUSBPort == INVALID_HANDLE_VALUE)
	{
		// 没有找到USB端口
		PostMessage(hDlg, UM_FINDPRINT, 1, 0);

		// 将错误信息写入日志文件
		WriteLogFile(_T("没有找到USB端口"), TRUE);

		return FALSE;
	}
	else
	{
		// 往打开的USB端口写入数据”1B 7C 00 04 00 49 b6 24 00“
		UINT uDataLen = 9;
		CHAR uData[9] = { 0x1B, 0x7C, 0x00, 0x04, 0x00, 0x49, 0xB6, 0x24, 0x00 };
		// 往打开的USB端口写入数据
		int iSendData = WritePort(NULL, uData, uDataLen, MS_TIMEOUT, hUSBPort);
		if (iSendData != -1)
		{
			// 关闭USB端口
			CloseUSBPort(hUSBPort);

			// 等待打印机重启
			Sleep(5000);
		}
		else
		{
			PostMessage(hDlg, UM_FINDPRINT, 1, 0);

			// 将错误信息写入日志文件
			WriteLogFile(_T("往打开的USB端口写入数据失败"), TRUE);

			return FALSE;
		}
	}
	return TRUE;
}

UINT ThreadFindGetID(LPVOID pParam)
{
	HWND hDlg = (HWND)pParam;
	PrintId* printID = new PrintId;
	CString strLinkPriter;
	CString strLinkCommand;
	CString strProduction;

	// 如果获取到标识为板载驱动，才执行下列发送数据的操作
	if (g_bOnboardDriver == 1)
	{
		// 新增发送初始化数据到USB端口的步骤
		BOOL bRet = SendInitializationDataToUSBPort(hDlg);
		if (!bRet)
		{
			// 发送指令失败，直接返回
			return 0;
		}
	}

	if (GetPrintID(strLinkPriter, strLinkCommand, strProduction, IDS_SelectPro))
	{
		//成功找到机器
		printID->strLinkPriter = strLinkPriter;
		printID->strLinkCommand = strLinkCommand;
		printID->strProduction = strProduction;
		PostMessage(hDlg, UM_FINDPRINT, 0, (LPARAM)printID);
	}
	else
	{
		//没有找到机器
		PostMessage(hDlg, UM_FINDPRINT, 1, 0);
	}

	return 0;
}

void CDlgFindPrint::OnCancel()
{
	// TODO:  在此添加专用代码和/或调用基类

	g_bKeepFind = FALSE;
	CDialogEx::OnCancel();
}

BOOL CDlgFindPrint::OnEraseBkgnd(CDC* pDC)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	CRect rect;
	GetClientRect(rect);
	CBrush brush = RGB(255, 255, 255);
	pDC->FillRect(rect, &brush);

	return true;
	//return CDialogEx::OnEraseBkgnd(pDC);
}

HBRUSH CDlgFindPrint::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (pWnd->GetDlgCtrlID() == IDC_STATIC1)
	{
		pDC->SetBkColor(RGB(255, 255, 255)); //设置字体背景为透明
		// TODO: Return a different brush if the default is not desired
		return (HBRUSH)::GetStockObject(WhiteColor);     // 设置背景色
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CDlgFindPrint::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO:  在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialogEx::OnPaint()
	//m_TopBanner.Load(m_hIns, IDB_BMP_MAINTOP);
	m_TopBanner.Load(m_hIns, IDB_MAINTOP, _T("PNG"));

	CRect RectMain;
	GetClientRect(RectMain);
	RectMain.bottom = RectMain.bottom / 4;
	RectMain.top += 1;
	RectMain.right -= 1;
	RectMain.left += 1;
	m_TopBanner.MoveWindow(RectMain, true);

	CRect rect;
	GetClientRect(&rect);
	CPen   pen(PS_SOLID, DlgFramePenSize, DlgFrameColor);   //在这里设置线型,线宽及颜色
	CPen* pOldPen = dc.SelectObject(&pen);

	//绘画对话框边框
	dc.MoveTo(CPoint(rect.left, rect.top));
	dc.LineTo(CPoint(rect.right, rect.top));
	dc.LineTo(CPoint(rect.right, rect.bottom));
	dc.LineTo(CPoint(rect.left, rect.bottom));
	dc.LineTo(CPoint(rect.left, rect.top));

	if (pOldPen)
	{
		dc.SelectObject(pOldPen);
	}
}

void CDlgFindPrint::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 1)
	{
		// 超过10秒仍未检测到打印机，则自动退出对话框
		KillTimer(1);
		EndDialog(IDCANCEL);
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CDlgFindPrint::SetTimeout(UINT nSeconds)
{
	// TODO: 在此处添加实现代码.
	m_nTimeoutSeconds = nSeconds;
}

void CDlgFindPrint::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}

void CDlgFindPrint::OnBnClickedbtncustominstall()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}
