#pragma once
#include "afxwin.h"
#include "MyButton.h"
#include "PictureCtrl.h"
//#include "resource.h"
#include "Setup.h"
// CDlgMesError 对话框

class CDlgMesError : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMesError)

public:
	CDlgMesError(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgMesError();

	// 对话框数据
	enum { IDD = IDD_DLG_MESERROR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	HINSTANCE m_hIns;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	CMyButton m_btnOk;
	afx_msg void OnBnClickedBtnErrorOk();
	CPictureCtrl m_TopBanner;

	CFont m_Font;
	CString m_strError;
	// 显示错误提示内容
	CEdit m_MesError;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
