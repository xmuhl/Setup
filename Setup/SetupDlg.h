// SetupDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "PictureCtrl.h"
#include "MyButton.h"
#include "DriversIni.h"
#include "afxcmn.h"
#include "TextProgressCtrl.h"
#include "Setup.h"
#include "Resource.h"
#include "PictureEx.h"
#include "CProgressDlg.h"

//#include <map>
using namespace std;

#define  UM_ENDSTALL			WM_USER+101		//�Զ�����Ϣ
#define  UM_ENDPROGBAR			WM_USER+104		// ����j������
#define WM_CLOSE_PROMPT_DLG_ONLINE (WM_USER + 1)
#define WM_UPDATE_PROMPT_TEXT (WM_USER + 2)
#define WM_CLOSE_PROMPT_DLG_OFFLINE (WM_USER + 3)

// CSetupDlg �Ի���
class CSetupDlg : public CDialogEx
{
	// ����
public:
	CSetupDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CSetupDlg();	// ��׼��������

	// �Ի�������
	enum { IDD = IDD_DLG_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	void PopulatePortListInView(CListCtrl& m_ListPort);
	// ʵ��
protected:
	HICON m_hIcon;
	HINSTANCE m_hIns;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);//����ƶ���Ϣ
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);//�����������Ϣ
	void InitializeFonts();
	afx_msg void OnBnClickedBtnPre();//�����һ����ť
	afx_msg void OnBnClickedBtnNext();//�����һ����ť
	afx_msg void OnBnClickedBtnCancle();//���ȡ����ť
	afx_msg void OnCbnSelchangeComboServer();//��ӡ��ϵ�иı����Ϣ
	afx_msg void OnCbnSelchangeComboModel();//��ӡ���ͺŸı����Ϣ
	afx_msg LRESULT OnEndStall(WPARAM   wParam, LPARAM   lParam);     // �Զ�����Ϣ ��Ӧ��װ������Ϣ
	afx_msg LRESULT OnEndProgressBar(WPARAM   wParam, LPARAM   lParam);     // �Զ�����Ϣ  ��ֹ������
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult);

	// �������
	CFont m_fontNormal;         // ��ͨ�ı�����
	CFont m_fontBold;           // ��������
	CFont m_fontTitle;          // ��������
	CFont m_fontLog;			// ��־ר������

	CFont m_ComboxFont;
	CFont m_InfoMesFont;

	// ϵ����Ͽ�
	CComboBox 		m_comboServer;
	// ������Ͽ�
	CComboBox  	m_comboModel;
	// ��һ����ť
	CMyButton m_btnPre;
	// ��һ����ť
	CMyButton m_btnNext;
	// ȡ����ť
	CMyButton m_btnCancel;

	//����ini�ļ������
	CDriversIni  m_Driver;

	// ��ӡ��ϵ������
	int m_iPrnServerCounts;
	//��Ŵ�ӡ��ϵ��
	Serives m_Seriver[SerivesMaxNum];
	//��Ŵ�ӡ�����ͺ�GPD
	PrinterDefInfo m_Printe[PrintMaxNum];

	// ����������
	CTextProgressCtrl m_Progress;

	// ��ʶ��ǰ��ʾ���ǵڼ���ҳ��
	int m_iPageCursel;

	//// ��ʾ�µĽ��棬���ؾɽ���
	void ShowPage(int showiPos, int beforeiPos);
	// ����1��ʾ�������غ���
	void Page1SHowOrHid(bool bMark);
	// ����2��ʾ�������غ���
	void Page2SHowOrHid(bool bMark);
	// ����3��ʾ�������غ���
	void Page3SHowOrHid(bool bMark);
	// ����4��ʾ�������غ���
	void Page4SHowOrHid(bool bMark);
	// ����5��ʾ�������غ���
	void Page5SHowOrHid(bool bMark);

	// ��ʼ����ӡ��ϵ�м��ͺ���Ͽ�
	void OnitPrintList(int iPosSer = -1, int iPosModel = -1);

	// ��ʼ���˿�
	BOOL GetConnectPort();

	//���صĴ�ӡ���ͺ��Ƿ��������б���
	BOOL PrintModelInList(CString PrintModel, int& iPosSer, int& iPosModel);

	//��ȡ��װ��Ϣ
	//SetUp *GetInstallInfo(CString LinkPriter, CString LinkCommand, CString  LinkProduction, CString SelServer, PrinterDefInfo info, BOOL bNewModel);
	SetUp* GetInstallInfo(CString LinkPriter, CString LinkCommand, CString  LinkProduction, CString SelServer, PrinterDefInfo info);

	// ��ŵ�ǰ���ӵĴ�ӡ���ͺ�
	CString m_strLinkPriter;
	// ��ʶ���ӵĻ����ķ���
	CString m_strLinkCommand;
	// ��ȡ�豸ID�Ĳ�������s
	CString  m_strLinkProduction;
	// ��ʶ�Ƿ�Ϊ��������
	BOOL m_bNewModel;
	// ��ǰ���ӵĴ�ӡ���˿ں�
	CString  m_strPort;

	// ֹͣ��װ���̱�ʶ
	BOOL m_bStopInstaller;

	SetUp* m_StructSetup; //����ڰ�װ��������ϸ��Ϣ

	// ��ӡ��ϵ�б�ǩ��
	CPictureCtrl m_PrnSerial;
	// ��ӡ���ͺű�ǩ��
	CPictureCtrl m_PrnModel;
	// �˿��б�
	CListCtrl m_ListPort;
	// ��ӡ��ϵ�б�ǩ���
	int m_PrnSerialID;
	// ��ӡ���ͺű����
	int m_PrnModelID;

	// ���浱ǰѡ����Ŀ���
	int m_PreSelItem;
	// ���ص�ַ��ά��
	CPictureCtrl m_Logo;
	// ��ʾ��Ϣ�ַ���
	CStatic m_strInfo;
	CRect  m_LinkRect;
	// ��װ������ʾ��ͼ	// ��װ������ʾ��ͼ
	CPictureEx m_SetupInfo;

	HACCEL m_hAccel;
	// ��С����ť
	CMyButton m_btnMini;
	// �رհ�ť
	CMyButton m_btnClose;
	afx_msg void OnBnClickedBtnMinimize();
	afx_msg void OnBnClickedBtnClose();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	void FindOemInfs(const CString& strPath, const CString& strPattern, CList<CString, CString&>& fileList);
	void UninstallDriverInf(const CString& driverFile);
	BOOL DeleteSelPrinterDriver(CString strPrnModel);
	BOOL CancelPendingPrintJobs(HANDLE hPrinter);
	CImage img;

	CList<CString, CString&> fileList;

	CProgressDlg* m_ProgressDlg;

	LRESULT OnClosePromptDialogOnLine(WPARAM wParam, LPARAM lParam);
	LRESULT OnClosePromptDialogOffLine(WPARAM wParam, LPARAM lParam);
};

//����������Ϣ���Ի洰��
void MesError(CString strError, BOOL bGetlastError = false);
BOOL StopPrintSpoolerService();
BOOL StartPrintSpoolerService();