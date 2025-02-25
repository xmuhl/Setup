#pragma once
#include "PictureCtrl.h"

// CDlgShowAd �Ի���

class CDlgShowAd : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgShowAd)

public:
	CDlgShowAd(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgShowAd();

	// �Ի�������
	enum { IDD = IDD_DLG_WELCOME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	// ������ʾ��λͼ
	void SetBitmap(UINT RecourceID);
	UINT m_RecourceID;  //�����ԴID
	virtual BOOL OnInitDialog();

	afx_msg void OnTimer(UINT_PTR nIDEvent);

	//  ��ʱ����ʱ��
	int m_Time;
	// ���ö�ʱ��ʱ��
	void SetShowTime(int Time);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	// ����������װ����
	CPictureCtrl m_PopUp_End;
};
//�رշ���
//UINT ClosePAV(LPVOID pParam);
