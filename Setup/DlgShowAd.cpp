// DlgShowAd.cpp : 实现文件
//

#include "stdafx.h"
#include "Setup.h"
#include "DlgShowAd.h"
#include "afxdialogex.h"
#include "UDefault.h"
#include "DebugStuff.h"
#include "PAVServiceOper.h"
// CDlgShowAd 对话框

IMPLEMENT_DYNAMIC(CDlgShowAd, CDialogEx)

CDlgShowAd::CDlgShowAd(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgShowAd::IDD, pParent)
	, m_Time(2000)
{
}

CDlgShowAd::~CDlgShowAd()
{
}

void CDlgShowAd::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PopUp_END, m_PopUp_End);
}

BEGIN_MESSAGE_MAP(CDlgShowAd, CDialogEx)

	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CDlgShowAd 消息处理程序
BOOL CDlgShowAd::OnEraseBkgnd(CDC* pDC)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	CBitmap m_bitmap;
	//添加图片
	m_bitmap.LoadBitmapW(m_RecourceID);

	CDC dcComp;
	dcComp.CreateCompatibleDC(pDC);
	dcComp.SelectObject(&m_bitmap);

	//设置缩放的大小模式，防止失真
	dcComp.SetStretchBltMode(COLORONCOLOR);

	BITMAP bmInfo;
	m_bitmap.GetObject(sizeof(bmInfo), &bmInfo);

	CRect rect;
	GetClientRect(rect);
	rect.bottom = rect.top + rect.Width() * bmInfo.bmHeight / bmInfo.bmWidth;

	pDC->StretchBlt(rect.left, rect.top, rect.Width(), rect.Height(), &dcComp, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY);

	return true;

	//return CDialogEx::OnEraseBkgnd(pDC);
}

// 设置显示的位图
void CDlgShowAd::SetBitmap(UINT RecourceID)
{
	m_RecourceID = RecourceID;
}

BOOL CDlgShowAd::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	//设置窗口大小
	CRect rtDesk;
	//::GetWindowRect(::GetDesktopWindow(), &rtDesk);
	::GetWindowRect(::GetDesktopWindow(), rtDesk);

	int desktopwidth = ::GetSystemMetrics(SM_CXSCREEN) - 1;
	int desktopheight = ::GetSystemMetrics(SM_CYSCREEN) - 1;

	int cx = 0;
	int cy = 0;
	//cx = 632* rtDesk.Width() / 1440;
	//cy = 356* rtDesk.Height() / 900;
	cx = ENDIMAGE_WIDTH * rtDesk.Width() / desktopwidth;
	cy = ENDIMAGE_HEIGHT * rtDesk.Height() / desktopheight;

	::SetWindowPos(this->m_hWnd, HWND_BOTTOM, 0, 0, cx, cy, SWP_NOZORDER);
	//::SetWindowPos(this->m_hWnd, HWND_BOTTOM, StartDlgRect, SWP_NOZORDER);

	//居中显示对话框
	CenterWindow();

	// 更改图像控件显示位置和大小
	//获取当前弹窗工作区大小
	CRect RectMain;
	GetClientRect(RectMain);
	RectMain.bottom -= 1;
	RectMain.top += 1;
	RectMain.right -= 1;
	RectMain.left += 1;
	m_PopUp_End.MoveWindow(RectMain, true);

	m_PopUp_End.LoadImageFromResource(IDB_END, _T("PNG"));

	::SetTimer(m_hWnd, 1, m_Time, NULL);

	//// 对话框屏幕居中  =
	//CRect rtDesk;
	//CRect rtDlg;
	//::GetWindowRect(::GetDesktopWindow(), &rtDesk);
	//GetWindowRect(&rtDlg);
	//int iXpos = rtDesk.Width() / 2 - rtDlg.Width() / 2;
	//int iYpos = rtDesk.Height() / 2 - rtDlg.Height() / 2;
	//SetWindowPos(FromHandle(HWND_TOPMOST), iXpos, iYpos, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

void CDlgShowAd::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	////只有程序启动画面
	//if (m_RecourceID == IDB_MP_START)
	//{
	//	//创建设子进程,关闭服务
	//	AfxBeginThread(ClosePAV, NULL, THREAD_PRIORITY_NORMAL);
	//}
	KillTimer(1);
	EndDialog(IDOK);
	CDialogEx::OnTimer(nIDEvent);
}

// 设置显示界面时间
void CDlgShowAd::SetShowTime(int Time)
{
	m_Time = Time;
}

BOOL CDlgShowAd::PreTranslateMessage(MSG* pMsg)
{
	// TODO:  在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		return TRUE;
	}
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}