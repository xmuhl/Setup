#pragma once

#include <WinSpool.h>

struct InstallFile          //Ԥ��װʱ��Ҫ���ļ���ϵ
{
	//���� _T("STDNAMES.GPD;UNIRES.DLL;START24RES.DLL;")
	LPTSTR DependentFiles;  //�����ļ�������
	LPTSTR CoverFile;  //���븲�ǵ��ļ�
	LPTSTR MustCopy;  //���뿽�����ļ�
};

//��ȡ��ӡ������ʱ����Ϣ
typedef struct
{
	CString strEnvironment;  //ϵͳ��ʶ
	CString strName;  //��������
	DWORD dwVersion;  //�汾��
}MY_DRIVER_INFO;

//ʹ��Ԥ��װģʽ��װ��ӡ������
/***************
������
DlghWnd �� �Ի�����
Seriver �� ϵ��
Model   �� �ͺ�
InstallName ����װ����
Port ���˿ں�
GPD ��GPD������
PointMark  ��������ʶ ��24�룺24Pin��12�룺12Pin��9�룺9Pin��������Thermal����תӡ��ThermalTran��
OsX64 ���Ƿ�Ϊ64λϵͳ
infName ����װ��������Inf����
������룺
0������ɹ�
1���ܾ������ļ�
2����װinf�ļ�ʧ��
3�����Ӵ�ӡ������ʧ��
4��д��ע���ʧ��
5�����Ӵ�ӡ��ʧ��
6����ΪĬ�ϴ�ӡ��ʧ��
***************/
int   PreInstalled(HWND DlghWnd, LPCTSTR Seriver, LPCTSTR Model, LPCTSTR InstallName, LPCTSTR Port, LPCTSTR GPD, LPCTSTR PointMark, BOOL OsX64, InstallFile File, LPCTSTR infName);

//ʹ������ģʽ��װ��ӡ������
/***************
������
DlghWnd �� �Ի�����
Model����ӡ���ͺ�
InstallName �� ��װ����������
PointMark  ��������ʶ ��24�룺24Pin��12�룺12Pin��9�룺9Pin��������Thermal����תӡ��ThermalTran��
GPD �� GPD�ļ�����
OsX64 �� �Ƿ�ΪX64ϵͳ
MFG ��������Ϣ������START ��������pnpid
bNewModel ���Ƿ�Ϊ��������
infName ����װ��������Inf����
����ֵ��
�µ�inf��·����
***************/
BOOL   OnlineInstalled(HWND DlghWnd, LPCTSTR Model, LPCTSTR InstallName, LPCTSTR PointMark, LPCTSTR GPD, BOOL OsX64, LPCTSTR  MFG, BOOL bNewModel, LPCTSTR infName);
//�������߼��ĵ���
/***************
������
ConfigFile �� ��װʱѡ��ı�ѡ���������߼����ֲ�ȣ�ÿ���ļ��Է������ ���磺_T("���߼�;�û��ֲ�;")
BootFolder �� ��Ŀ¼�ļ�������
������룺
�ɹ���TRUE
ʧ�ܣ�FALSE
***************/
//BOOL   CopyToolAndDoc(LPCTSTR FileName, LPCTSTR BootFolder);
// // //�ж�.NET Framework 4.0�Ƿ�װ
// // /***************
// // ����ֵ��
// // �Ѱ�װ��TRUE
// // δ��װ��FALSE
// // ***************/
// // BOOL  IsInstallNet40();
// //
// //���а�װ.net4.0
// /***************
// ������
// Weblink ����ҳ���ӣ�Ŀǰ����  http://shidaapp.duapp.com/Down/Microsoft.NET.exe"
// ����ֵ��
// �Ѱ�װ��TRUE
// δ��װ��FALSE
// ***************/
// void  SetupNet4(LPTSTR Weblink = _T("http://shidaapp.duapp.com/Down/Microsoft.NET.exe"));
//
//
//
//�ж�ϵͳ�Ƿ����д˴�ӡ������
/***************
������
InstallName �� ��װ����������
����ֵ��
�Ѱ�װ��TRUE
δ��װ��FALSE
***************/
//BOOL   IsHavePrint(LPCTSTR InstallName);
//��ȡ��ӡ��������Ϣ
/***************
������
Model �� �����ͺ�
����ֵ��
my_die :�Զ���ṹ��
***************/
MY_DRIVER_INFO* GetPrintDriverInfo(LPCTSTR Model);
//ж�ش�ӡ������
/***************
������
my_die :�Զ���ṹ��
InstallName �� ��װ����������
����ֵ��
ж�سɹ���TRUE
ж��ʧ�ܣ�FALSE
***************/
//BOOL  UninstallDriver(LPCTSTR InstallName, MY_DRIVER_INFO *die);

//ͨ��Usb�ڻ�ȡ������Ϣ���豸���ƣ������
/***************
������
PrintName :��ӡ���ͺ�
Provider ����������
Command �� ����ָ��
SelectPro�� ɸѡ���� ����_T("START")
����ֵ��
��ȡ�ɹ���TRUE
��ȡʧ�ܣ�FALSE
***************/
BOOL  GetUSBPrintId(LPTSTR& PrintName, LPTSTR& Provider, LPTSTR& Command, LPTSTR SelectPro = _T(""));

/*
*�������ܣ�
*
//��ȡ��ӡ���豸ID������
*
*����������
*
*		HANDLE hPort �豸���
LPTSTR Model �豸�ͺ�
LPTSTR Production   ������Ϣ
LPTSTR Simulation  ������Ϣ
SelectPro�� ɸѡ���� ����_T("START"),Ϊ����Ϊ��ɸѡ
*
����ֵ��
��ȡ�ɹ���TRUE
��ȡʧ�ܣ�FALSE
*
*��������ֵ��
*
*/
BOOL  GetPrintID_LPT(LPTSTR& Model, LPTSTR& Production, LPTSTR& Simulation, LPTSTR SelectPro = _T(""));

//���ҵ�ǰ���ӵĴ�ӡ����USB�˿ں�
/***************
������
VIDPID :VIDPID ɸѡ����  ���磺_T("vid_xxxx&pid_xxxx")  x:0-9����
����ֵ��
δ�ҵ��˿ڷ��� 0�������򷵻ض˿ں���USB001 = 1
***************/
int  FindUSBPortID(LPCTSTR VIDPID);

//ö�����ж˿�
/***************
����ֵ��
LPTSTR�����ж˿����ƣ�ÿ���˿������ԣ�����
***************/
LPTSTR   EnumerateAllPort();
//����������ϵͳCFont·����
//BOOL  CopyFont();

//�رշ���
//BOOL  CloseSampleService(LPTSTR Service);
//��������
//BOOL  StartSampleService(LPTSTR Service);
//����ж�س���
//BOOL  ShellUninstallExe(LPTSTR AppPath = _T(""));
//ɨ���豸������
void  ScanForHardwareChanges1();

//BOOL  ChangePrinterHighAttributes(LPCTSTR lpDeviceName, bool bOpen);

//ö�����д�ӡ������
int EnumAllPrintDriver(DRIVER_INFO_6*& die, DWORD& dSize);