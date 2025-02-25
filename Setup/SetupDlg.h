// SetupDlg.h : 头文件
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

#define  UM_ENDSTALL			WM_USER+101		//自定义消息
#define  UM_ENDPROGBAR			WM_USER+104		// 结束j进度条
#define WM_CLOSE_PROMPT_DLG_ONLINE (WM_USER + 1)
#define WM_UPDATE_PROMPT_TEXT (WM_USER + 2)
#define WM_CLOSE_PROMPT_DLG_OFFLINE (WM_USER + 3)

// CSetupDlg 对话框
class CSetupDlg : public CDialogEx
{
	// 构造
public:
	CSetupDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CSetupDlg();	// 标准析构函数

	// 对话框数据
	enum { IDD = IDD_DLG_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	void PopulatePortListInView(CListCtrl& m_ListPort);
	// 实现
protected:
	HICON m_hIcon;
	HINSTANCE m_hIns;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);//鼠标移动消息
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);//鼠标左键点击消息
	void InitializeFonts();
	afx_msg void OnBnClickedBtnPre();//点击上一步按钮
	afx_msg void OnBnClickedBtnNext();//点击下一步按钮
	afx_msg void OnBnClickedBtnCancle();//点击取消按钮
	afx_msg void OnCbnSelchangeComboServer();//打印机系列改变的消息
	afx_msg void OnCbnSelchangeComboModel();//打印机型号改变的消息
	afx_msg LRESULT OnEndStall(WPARAM   wParam, LPARAM   lParam);     // 自定义消息 响应安装结束信息
	afx_msg LRESULT OnEndProgressBar(WPARAM   wParam, LPARAM   lParam);     // 自定义消息  终止进度条
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult);

	// 字体对象
	CFont m_fontNormal;         // 普通文本字体
	CFont m_fontBold;           // 粗体字体
	CFont m_fontTitle;          // 标题字体
	CFont m_fontLog;			// 日志专用字体

	CFont m_ComboxFont;
	CFont m_InfoMesFont;

	// 系列组合框
	CComboBox 		m_comboServer;
	// 机型组合框
	CComboBox  	m_comboModel;
	// 上一步按钮
	CMyButton m_btnPre;
	// 下一步按钮
	CMyButton m_btnNext;
	// 取消按钮
	CMyButton m_btnCancel;

	//访问ini文件类对象
	CDriversIni  m_Driver;

	// 打印机系列数量
	int m_iPrnServerCounts;
	//存放打印机系列
	Serives m_Seriver[SerivesMaxNum];
	//存放打印机机型和GPD
	PrinterDefInfo m_Printe[PrintMaxNum];

	// 进度条变量
	CTextProgressCtrl m_Progress;

	// 标识当前显示的是第几个页面
	int m_iPageCursel;

	//// 显示新的界面，隐藏旧界面
	void ShowPage(int showiPos, int beforeiPos);
	// 界面1显示或者隐藏函数
	void Page1SHowOrHid(bool bMark);
	// 界面2显示或者隐藏函数
	void Page2SHowOrHid(bool bMark);
	// 界面3显示或者隐藏函数
	void Page3SHowOrHid(bool bMark);
	// 界面4显示或者隐藏函数
	void Page4SHowOrHid(bool bMark);
	// 界面5显示或者隐藏函数
	void Page5SHowOrHid(bool bMark);

	// 初始化打印机系列及型号组合框
	void OnitPrintList(int iPosSer = -1, int iPosModel = -1);

	// 初始化端口
	BOOL GetConnectPort();

	//返回的打印机型号是否在现有列表中
	BOOL PrintModelInList(CString PrintModel, int& iPosSer, int& iPosModel);

	//获取安装信息
	//SetUp *GetInstallInfo(CString LinkPriter, CString LinkCommand, CString  LinkProduction, CString SelServer, PrinterDefInfo info, BOOL bNewModel);
	SetUp* GetInstallInfo(CString LinkPriter, CString LinkCommand, CString  LinkProduction, CString SelServer, PrinterDefInfo info);

	// 存放当前连接的打印机型号
	CString m_strLinkPriter;
	// 标识连接的机器的仿真
	CString m_strLinkCommand;
	// 获取设备ID的产家名称s
	CString  m_strLinkProduction;
	// 标识是否为新增机型
	BOOL m_bNewModel;
	// 当前连接的打印机端口号
	CString  m_strPort;

	// 停止安装进程标识
	BOOL m_bStopInstaller;

	SetUp* m_StructSetup; //存放在安装的驱动详细信息

	// 打印机系列标签名
	CPictureCtrl m_PrnSerial;
	// 打印机型号标签名
	CPictureCtrl m_PrnModel;
	// 端口列表
	CListCtrl m_ListPort;
	// 打印机系列标签序号
	int m_PrnSerialID;
	// 打印机型号标序号
	int m_PrnModelID;

	// 保存当前选中项目序号
	int m_PreSelItem;
	// 下载地址二维码
	CPictureCtrl m_Logo;
	// 提示信息字符串
	CStatic m_strInfo;
	CRect  m_LinkRect;
	// 安装进度提示动图	// 安装进度提示动图
	CPictureEx m_SetupInfo;

	HACCEL m_hAccel;
	// 最小化按钮
	CMyButton m_btnMini;
	// 关闭按钮
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

//弹出错误信息，自绘窗口
void MesError(CString strError, BOOL bGetlastError = false);
BOOL StopPrintSpoolerService();
BOOL StartPrintSpoolerService();