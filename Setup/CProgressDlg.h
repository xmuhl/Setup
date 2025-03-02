﻿#pragma once
#include "afxdialogex.h"
#include "PictureEx.h"

#define WhiteColor  RGB(255,255,255)

// CProgressDlg 对话框

class CProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CProgressDlg)

public:
	CProgressDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CProgressDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_PROGRESS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	CPictureEx  m_animation;
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

private:
	CRect m_AnimateRect;  // 用于保存动画控件的位置
	CSize m_GifSize;      // 用于保存GIF图片的实际大小
};
