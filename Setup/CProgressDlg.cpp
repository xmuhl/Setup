// CProgressDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Setup.h"
#include "afxdialogex.h"
#include "CProgressDlg.h"

// CProgressDlg 对话框

IMPLEMENT_DYNAMIC(CProgressDlg, CDialogEx)

CProgressDlg::CProgressDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_PROGRESS, pParent)
{
}

CProgressDlg::~CProgressDlg()
{
	m_animation.UnLoad();
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANIMATION, m_animation);
}

BEGIN_MESSAGE_MAP(CProgressDlg, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CProgressDlg 消息处理程序
BOOL CProgressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 获取动画控件和窗口尺寸
	CRect dlgRect, animRect;
	GetClientRect(&dlgRect);
	m_animation.GetWindowRect(&animRect);
	ScreenToClient(&animRect);

	// 预定义变量
	int x = 0;
	int y = 0;
	SIZE gifSize = { 0, 0 };

	// 加载GIF
	if (m_animation.Load(MAKEINTRESOURCE(IDR_GIF1), _T("gif")))
	{
		// 获取GIF尺寸
		gifSize = m_animation.GetSize();

		// 计算居中位置
		x = (dlgRect.Width() - gifSize.cx) / 2;
		y = (dlgRect.Height() - gifSize.cy) / 2;

		// 调整控件位置以居中显示GIF
		m_animation.MoveWindow(x, y, gifSize.cx, gifSize.cy);

		// 启动动画
		m_animation.Draw();
	}

	// 获取标签控件
	CWnd* pLabel = GetDlgItem(IDC_PROGRESS_LABEL);
	if (pLabel)
	{
		CRect textRect;
		pLabel->GetWindowRect(&textRect);
		ScreenToClient(&textRect);

		// 设置标签位置
		pLabel->MoveWindow(
			dlgRect.Width() / 2 - textRect.Width() / 2,  // 水平居中
			y + gifSize.cy + 20,  // GIF下方10像素
			textRect.Width(),
			textRect.Height()
		);

		// 设置标签文本
		SetDlgItemText(IDC_PROGRESS_LABEL, _T("发现旧版驱动文件，正在自动删除..."));
	}

	return TRUE;
}

//BOOL CProgressDlg::OnInitDialog()
//{
//	CDialogEx::OnInitDialog();
//
//	// TODO:  在此添加额外的初始化
//	if (m_animation.Load(MAKEINTRESOURCE(IDR_GIF1), _T("gif")))
//	{
//		m_animation.Draw();
//	}
//
//	// 在这里设置初始标签文本
//	SetDlgItemText(IDC_PROGRESS_LABEL, _T("发现旧版驱动文件，正在自动删除..."));
//
//	return TRUE;  // return TRUE unless you set the focus to a control
//	// 异常: OCX 属性页应返回 FALSE
//}

BOOL CProgressDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rect;
	GetClientRect(rect);
	CBrush brush = RGB(255, 255, 255);
	pDC->FillRect(rect, &brush);

	return true;

	//return CDialogEx::OnEraseBkgnd(pDC);
}

HBRUSH CProgressDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (pWnd->GetDlgCtrlID() == IDC_PROGRESS_LABEL)
	{
		pDC->SetBkColor(RGB(255, 255, 255)); //设置字体背景为透明
		// TODO: Return a different brush if the default is not desired
		return (HBRUSH)::GetStockObject(WhiteColor);     // 设置背景色
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}