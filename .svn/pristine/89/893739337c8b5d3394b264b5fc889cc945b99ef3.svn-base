// DlgMesError.cpp : 实现文件
//

#include "stdafx.h"
#include "Setup.h"
#include "DlgMesError.h"
#include "afxdialogex.h"
#include "UDefault.h"

// CDlgMesError 对话框
IMPLEMENT_DYNAMIC(CDlgMesError, CDialogEx)

CDlgMesError::CDlgMesError(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgMesError::IDD, pParent)
	, m_strError(_T(""))
{
	m_hIns = AfxGetInstanceHandle();
}

CDlgMesError::~CDlgMesError()
{
}

void CDlgMesError::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_ERROR_OK, m_btnOk);
	DDX_Control(pDX, IDC_TOP__BANNER, m_TopBanner);
	DDX_Control(pDX, IDC_EDIT_MesError, m_MesError);
}

BEGIN_MESSAGE_MAP(CDlgMesError, CDialogEx)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_ERROR_OK, &CDlgMesError::OnBnClickedBtnErrorOk)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// CDlgMesError 消息处理程序

BOOL CDlgMesError::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	//居中显示对话框
	CenterWindow();

	// TODO:  在此添加额外的初始化
	//HINSTANCE  hIns = AfxGetInstanceHandle();
	//m_TopBanner.Load(hIns, IDB_BMP_MAINTOP);

	// 设置提示字符串字体
	m_Font.CreateFont(0, 0, 0, 0, FW_THIN,			//FW_NORMAL,
		FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("黑体"));

	//m_Font.CreateFont(0,0, 0, 0, FW_THIN, false, false, false,
	//	CHINESEBIG5_CHARSET, OUT_CHARACTER_PRECIS,
	//	CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
	//	FF_MODERN, _T("黑体"));

	CString strMessErr;
	strMessErr = _T("驱动安装失败！\r\n\r\n") + m_strError;		// 尝试以下解决方案：\r\n\r\n...
	//strMessErr += _T("\r\n\r\n如错误情况依然存在，请联系设备厂家获取进一步的技术支持。\r\n");
	m_MesError.SetFont(&m_Font);
	m_MesError.SetWindowTextW(strMessErr);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

void CDlgMesError::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO:  在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialogEx::OnPaint()

	//HINSTANCE  hIns = AfxGetInstanceHandle();
	//m_TopBanner.Load(m_hIns, IDB_BMP_MAINTOP);
	// 加载弹出对话框顶部横幅
	//m_TopBanner.Load(m_hIns, IDB_BMP_MAINTOP);
	m_TopBanner.Load(m_hIns, IDB_MAINTOP, _T("PNG"));

	CRect RectMain;
	GetClientRect(RectMain);
	RectMain.bottom = RectMain.bottom / 4;
	RectMain.top += 1;
	RectMain.right -= 1;
	RectMain.left += 1;
	m_TopBanner.MoveWindow(RectMain, true);

	//CBrush Brush(RGB(255, 255, 255));
	//dc.FillRect(rect, &Brush);
	CRect rect;
	GetClientRect(&rect);
	CPen   pen(PS_SOLID, DlgFramePenSize, DlgFrameColor);   //在这里设置线型,线宽及颜色
	CPen* pOldPen = dc.SelectObject(&pen);

	////绘画对话框边框
	//CRect rect;
	//CPaintDC dc(this); // 用于绘制的设备上下文
	//GetClientRect(&rect);
	//CPen   pen(PS_SOLID, DlgFramePenSize, DlgFrameColor);   //在这里设置线型,线宽及颜色
	//CPen* pOldPen = dc.SelectObject(&pen);

	//绘画对话框边框
	dc.MoveTo(CPoint(rect.left, rect.top));
	dc.LineTo(CPoint(rect.right, rect.top));
	dc.LineTo(CPoint(rect.right, rect.bottom));
	dc.LineTo(CPoint(rect.left, rect.bottom));
	dc.LineTo(CPoint(rect.left, rect.top));

	//CFont * lpOldFont = dc.SelectObject(&m_Font);
	//
	//CString strError = _T("原因及解决：");
	//int iTop = 118; //第一行坐标
	//int iLeng = m_strError.GetLength();
	//int RowNum = 21; //一行的个数
	//if (iLeng <= RowNum)
	//{
	//	//一行显示
	//	strError += m_strError;
	//	dc.TextOutW(50, iTop, strError);
	//	iTop += RowNum;
	//}
	//else
	//{
	//	//第一行显示
	//	strError += m_strError.Left(RowNum);
	//	dc.TextOutW(50, iTop, strError);
	//	iTop += RowNum;

	//	int iRow = iLeng / RowNum;
	//	int remainder = iLeng % RowNum; //余数
	//	for (int i = 1; i < iRow; i++)
	//	{
	//		CString str = _T("");
	//		str = m_strError.Mid(RowNum * i, RowNum);
	//		dc.TextOutW(115, iTop, str);
	//		iTop += RowNum;
	//	}
	//	//输出最后一行
	//	if (remainder)
	//	{
	//		CString str = _T("");
	//		str = m_strError.Mid(RowNum * iRow, RowNum);
	//		dc.TextOutW(110, iTop, str);
	//		iTop += RowNum;
	//	}
	//	CRect rect1;
	//	rect1.left = 150;
	//	rect1.right = 257;
	//	rect1.top = iTop + 60;
	//	rect1.bottom = iTop + 96;
	//	m_btnOk.MoveWindow(rect1, FALSE);
	//}

	//dc.TextOutW(50, iTop+10, _T("请关闭所有应用程序，并重启操作系统后再次运行安装程序。"));
	//dc.TextOutW(50, iTop+30, _T("如错误情况依然存在，请联系设备厂家获取进一步的技术支持。"));

	if (pOldPen)
	{
		dc.SelectObject(pOldPen);
	}
	//if (lpOldFont)
	//{
	//	dc.SelectObject(lpOldFont);
	//}
}

void CDlgMesError::OnBnClickedBtnErrorOk()
{
	// TODO:  在此添加控件通知处理程序代码
	EndDialog(IDOK);
}

HBRUSH CDlgMesError::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_MesError)
	{
		pDC->SetBkColor(RGB(255, 255, 255)); //设置字体背景为透明
		// TODO: Return a different brush if the default is not desired
		return (HBRUSH)::GetStockObject(WhiteColor);     // 设置背景色
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

BOOL CDlgMesError::OnEraseBkgnd(CDC* pDC)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	CRect rect;
	GetClientRect(rect);
	CBrush brush = RGB(255, 255, 255);
	pDC->FillRect(rect, &brush);

	return true;

	//return CDialogEx::OnEraseBkgnd(pDC);
}

void CDlgMesError::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	//实现对话框跟随鼠标移动
	::SendMessage(GetSafeHwnd(), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);

	CDialogEx::OnLButtonDown(nFlags, point);
}