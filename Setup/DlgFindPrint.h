#pragma once
#include "PictureEx.h"
#include "afxwin.h"
#include "Setup.h"
#include "UDefault.h"
#include "PictureCtrl.h"
// CDlgFindPrint 对话框
#define UM_FINDPRINT  WM_USER+104 //找到了打印机
class CDlgFindPrint : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgFindPrint)

public:
	CDlgFindPrint(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgFindPrint();

	// 对话框数据
	enum { IDD = IDD_DLG_FINDPRINTER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	HINSTANCE m_hIns;
public:
	CPictureEx m_Pic;
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnFindPrint(WPARAM   wParam, LPARAM   lParam);
	CString m_strLinkPriter;
	CString m_strLinkCommand;
	CString m_strLinkProduction;

	virtual void OnCancel();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	// 设备连接提示信息
	CStatic m_strConnectMes;
	afx_msg void OnPaint();
	//
	CPictureCtrl m_TopBanner;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
	// 超时时间(秒)
	UINT m_nTimeoutSeconds;
public:
	void SetTimeout(UINT nSeconds);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedbtncustominstall();
};

// 获取设备ID
BOOL GetPrintID(CString& strLinkPriter, CString& strLinkCommand, CString& strProduction, LPTSTR SelectPro);
UINT ThreadFindGetID(LPVOID pParam);
