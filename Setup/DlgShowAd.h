#pragma once
#include "PictureCtrl.h"

// CDlgShowAd 对话框

class CDlgShowAd : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgShowAd)

public:
	CDlgShowAd(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgShowAd();

	// 对话框数据
	enum { IDD = IDD_DLG_WELCOME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	// 设置显示的位图
	void SetBitmap(UINT RecourceID);
	UINT m_RecourceID;  //存放资源ID
	virtual BOOL OnInitDialog();

	afx_msg void OnTimer(UINT_PTR nIDEvent);

	//  定时器的时间
	int m_Time;
	// 设置定时器时间
	void SetShowTime(int Time);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	// 弹出结束安装画面
	CPictureCtrl m_PopUp_End;
};
//关闭服务
//UINT ClosePAV(LPVOID pParam);
