/************************************************************************

FileName:	LPTCtrl.h

Copyright (c) 2009-2012

Writer:	����

create Date:	202.07.10

Rewriter:

Rewrite Date:

Impact:	LPT�˿ڿ��ƹ���ģ��Ԥ����ͷ�ļ�������

Main Content:
GetLPTDeviceID();			// ��ȡ�����豸ID
OpenLPT();						// �򿪲���
WriteLPTPort();					// ������д������
CloseLPTPort();				// �رղ��ھ��

/************************************************************************/

#include "stdafx.h"

#include "LPTCtrl.h"
#include <setupapi.h>
#include "DebugStuff.h"

#include <WinIoCtl.h>
#include "ntddpar.h"
//#include "uConstDef.h"
#include <cfgmgr32.h>
#include "StrTranslate.h"
#pragma comment (lib,"setupapi.lib")

#define GUID_DEVINTERFACE_PARALLEL {0x97F76EF0, 0xF883, 0x11D0, {0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C}}
#define  LOOP_COUNTS	3				// ѭ���жϴ���
#define  SLEEP_TIME		0			// Ӳ��ɨ����ʱ��

// �򿪲���
HANDLE OpenLPTPort(CString PortNum)
{
	HANDLE hLPTPort = INVALID_HANDLE_VALUE;				// ��ʼ��һ����Ч���

	CString Port = QueryLPTPort(PortNum);

	CString strPort = _T("\\\\.\\");
	strPort += Port;

	hLPTPort = CreateFile(strPort,			 // interfaceDetail->DevicePath
		GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	//hLPTPort = CreateFile(_T("\\\\.\\LPT1"),			 // interfaceDetail->DevicePath
	//	GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	return hLPTPort;
}
//ö���豸������,���ش򿪵Ĳ��ڣ�������豸ID���ؿ���Ϊ1.4...
CString QueryLPTPort(CString PortNum)
{
	CString strReturn = _T("");
	// ��ȡ��ǰ���ڶ˿��б�
	TCHAR GUID[20] = { 0 };
	DWORD dwGUIDsize;
	DWORD dwsize = 0;
	BOOL bResult;
	int iLPTID = 0;

	bResult = SetupDiClassGuidsFromName(_T("Ports"), (LPGUID)GUID, 20, &dwGUIDsize);

	if (bResult)
	{
		//Get device info set for device class (SetupDiGetClassDevsA function)
		HDEVINFO					hardwareDeviceInfo;		// �豸��Ϣ�����ݽṹ

		hardwareDeviceInfo = SetupDiGetClassDevs((LPGUID)GUID,
			NULL, NULL, (DIGCF_PRESENT)); // ����ָ����USB�豸��GUID��ȡ�豸��Ϣ��

		if (hardwareDeviceInfo == INVALID_HANDLE_VALUE)			// �޷���ȡָ���豸���豸����Ϣ������������ʾ��
		{
			GetErrAndLog(_T("�޷���ȡָ���豸��Ϣ��!"));
		}
		else
		{
			//Get device info data for every device (SetupDiGetClassDevsA function, second parameter for this function is sequential device index in the device class, so call this function in circle with device index = 0, 1, etc.).
			SP_DEVINFO_DATA				deviceInfoData;			// ָ���ӿڵ��豸��Ϣ���ݽṹ
			deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

			DWORD dwPortID = 0;
			BOOL bGetPortInfo = FALSE;
			do
			{
				bGetPortInfo = SetupDiEnumDeviceInfo(hardwareDeviceInfo, dwPortID, &deviceInfoData);

				if (bGetPortInfo)
				{
					DWORD dwProperty = 0;
					DWORD dwRequireSize = 0;
					BYTE Propbuf[MAX_PATH] = { 0 };
					CString strPortname;

					BOOL bGetRegInfo = SetupDiGetDeviceRegistryProperty(hardwareDeviceInfo, &deviceInfoData,
						SPDRP_FRIENDLYNAME, &dwProperty, Propbuf, MAX_PATH, &dwRequireSize);

					if (bGetRegInfo)
					{
						CString strDevicename = (LPCTSTR)Propbuf;

						if (strDevicename.Find(PortNum) != -1)
						{
							strPortname = strDevicename.Mid(strDevicename.Find(_T("(LPT")) + 1, 4);

							TCHAR* szMsg = new TCHAR[100];
							//�����ΪCString�ַ����ĳ���
							szMsg = strPortname.GetBuffer(strPortname.GetLength());

							strPortname.ReleaseBuffer();

							//���ú����жϸö˿��Ƿ����ӵ���ʵ���ӡ��,���ش�ӡ���ͺ�,û����Ϊ��
							//IsSTDeviceID(szMsg, strReturn);

							strReturn = ResolveLPTPort(szMsg);
							iLPTID++;

							break;
						}
					}
				}
				dwPortID++;
			} while (bGetPortInfo);
		}
		SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
	}
	else
	{
		GetErrAndLog(_T("��ȡ�˿�GUIDʧ�ܣ�"));
	}

	return strReturn;
}
//��LPT��ȡ������ID��������ӡ�����ں�
CString ResolveLPTPort(CString PrintId)
{
	CString strReturn = _T("");
	int iPos = PrintId.ReverseFind('&');
	int iBufSize = PrintId.GetLength();
	if ((iPos != -1) && (iPos <= iBufSize))
	{
		strReturn = PrintId.Mid(iPos + 1, iBufSize - iPos);
	}
	return strReturn;
}
BOOL   ScanForHardwareChanges(TCHAR* pDeviceID)
{
	DEVINST           devInst;
	CONFIGRET       status;

	//
	//   Get   the   root   devnode.
	//

	status = CM_Locate_DevNode(&devInst, pDeviceID, CM_LOCATE_DEVNODE_NORMAL);

	if (status != CR_SUCCESS)
	{
		//GetErrAndLog(_T("CM_Locate_DevNode   failed:   %x\n ") + status, TRUE);
		CString strError;
		strError.Format(_T("CM_Locate_DevNode failed: %x\n"), status);
		GetErrAndLog(strError);

		return   FALSE;
	}

	status = CM_Reenumerate_DevNode(devInst, 0);

	if (status != CR_SUCCESS)
	{
		CString strError;
		strError.Format(_T("CM_Reenumerate_DevNode failed: %x\n"), status);
		GetErrAndLog(strError);
		//GetErrAndLog(_T("CM_Reenumerate_DevNode   failed:   %x\n ") + status, TRUE);
		return   FALSE;
	}

	return   TRUE;
}
// ��ȡ�����豸ID
BOOL IsSTDeviceID(TCHAR* lpLPTName, CString& PrintModel)
{
	BOOL bIsSTDevice = FALSE;

	// ͨ������GUID��ʼ�����豸�ӿ���ϸ��Ϣ
	GUID ClassGuid = GUID_DEVINTERFACE_PARALLEL;
	LPGUID  pGuid = &ClassGuid;

	HDEVINFO devs;
	devs = SetupDiGetClassDevs(pGuid, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	SP_DEVINFO_DATA devInfo;
	SP_DEVICE_INTERFACE_DATA devInterface;
	DWORD size;

	PSP_DEVICE_INTERFACE_DETAIL_DATA interfaceDetail = NULL;

	if (devs == INVALID_HANDLE_VALUE)
	{
		GetErrAndLog(_T("SetupDiGetClassDevs!"));
		return 0;
	}

	DWORD devCount = 0;
	devInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	while (SetupDiEnumDeviceInterfaces(devs, 0, pGuid, devCount, &devInterface))
	{ //1
		devCount++;
		size = 0;

		SetupDiGetDeviceInterfaceDetail(devs, &devInterface, 0, 0, &size, 0);

		devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
		interfaceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)calloc(1, size);

		if (interfaceDetail)
		{ //2
			interfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			devInfo.cbSize = sizeof(SP_DEVINFO_DATA);

			if (!SetupDiGetDeviceInterfaceDetail(devs, &devInterface, interfaceDetail, size, 0, &devInfo))
			{  //3
				free(interfaceDetail);
				SetupDiDestroyDeviceInfoList(devs);
				GetErrAndLog(_T("SetupDiGetDeviceInterfaceDetail"));
				return 0;
			} // 3//

			// �ж��Ƿ���ָ���Ĳ���,�����ж������
			TCHAR  driverkey[MAX_PATH] = { 0 };
			DWORD dataType;
			if (!SetupDiGetDeviceRegistryProperty(devs, &devInfo, SPDRP_FRIENDLYNAME,
				&dataType, (LPBYTE)driverkey, size, 0))
			{// 3
				free(interfaceDetail);
				SetupDiDestroyDeviceInfoList(devs);

				GetErrAndLog(_T("SetupDiGetDeviceRegistryProperty"));
				return FALSE;
			}
			else
			{
				_wcsupr_s(driverkey);
				//DebugMesBox(driverkey);

				if (!wcsstr(driverkey, lpLPTName))
				{ // 4
					//SetupDiDestroyDeviceInfoList(devs);
					continue;
					//return 0;
				}	//4
			}//3

			for (int i = 0; i < LOOP_COUNTS; i++)		//���ɨ��Ӳ��
			{//3
				// ɨ��Ӳ���Ķ�
				// Get device ID string length
				ULONG ulSize = 0;
				if (CM_Get_Device_ID_Size(&ulSize, devInfo.DevInst, 0) == CR_SUCCESS)
				{ // 4
					// Get first child device DeviceID
					TCHAR* strDeviceID = new TCHAR[ulSize + 1];
					if (CM_Get_Device_ID(devInfo.DevInst, strDeviceID, ulSize + 1, 0) == CR_SUCCESS)
					{ //5
						//DebugMesBox(strDeviceID);
						// ɨ��Ӳ���Ķ�
						if (ScanForHardwareChanges(strDeviceID) != TRUE)
						{ //6
							delete[] strDeviceID;
							strDeviceID = NULL;

							free(interfaceDetail);
							SetupDiDestroyDeviceInfoList(devs);
							GetErrAndLog(_T("ScanForHardwareChanges Failure"));
							return FALSE;
						}	// 6//
						delete[] strDeviceID;
						strDeviceID = NULL;
					} // 5 //
				}//4//

				//��ȡָ���������߾������ʼö�ٵ�ǰ�����������ӵ����豸���Ƿ���ָ���ͺ�
				ULONG hdevChild;
				// Get Child device handle
				if (CM_Get_Child(&hdevChild, devInfo.DevInst, 0) == CR_SUCCESS)
				{//4
					if (CM_Get_Device_ID_Size(&ulSize, hdevChild, 0) == CR_SUCCESS)
					{//5
						bIsSTDevice = FALSE;      // ���¿�ʼɨ�裬��ʶλ����

						// Get first child device DeviceID
						TCHAR* strDeviceID = new TCHAR[ulSize + 1];
						if (CM_Get_Device_ID(hdevChild, strDeviceID, ulSize + 1, 0) == CR_SUCCESS)
						{//6
							//DebugMesLog(strDeviceID);
							delete[] strDeviceID;
							strDeviceID = NULL;

							//��ʼö�ٺ������豸
							CONFIGRET cr;
							do
							{//7
								cr = CM_Get_Sibling(&hdevChild, hdevChild, 0);
								if (cr == CR_SUCCESS)
								{//8
									if (CM_Get_Device_ID_Size(&ulSize, hdevChild, 0) == CR_SUCCESS)
									{//9
										TCHAR* strDeviceID = new TCHAR[ulSize + 1];

										if (CM_Get_Device_ID(hdevChild, strDeviceID, ulSize + 1, 0) == CR_SUCCESS)
										{//10
											//DebugMesLog(strDeviceID);
											if (wcsstr(strDeviceID, STR_DEVICE_ID))
											{//11
												PrintModel = strDeviceID;
												bIsSTDevice = TRUE;					// �ҵ�ָ���豸ID

												delete[] strDeviceID;
												strDeviceID = NULL;

												break;
											}//11//
										}//10//
										delete[] strDeviceID;
										strDeviceID = NULL;
									}//9//
								}//8//
							} while (cr == CR_SUCCESS);//7//
						}//6//
					}//5//
				}//4//

				if (bIsSTDevice)	// �ҵ�ָ���豸ID���˳�ѭ�� ��ѯ
				{//4
					break;
					//Sleep(SLEEP_TIME);
				}
				else				// ����ֹͣ3���ӣ��ȴ���ӡ����λ���ٴβ�ѯ
				{
					Sleep(SLEEP_TIME);
					/*bIsSTDevice = FALSE;
					break;				*/
				}//4
			}	// 3 //���Ӳ��ɨ�����
		} //2

		free(interfaceDetail);
	}// 1

	//free(interfaceDetail);
	SetupDiDestroyDeviceInfoList(devs);
	return bIsSTDevice;
}

//��USB��ȡ������ID��������ӡ���ͺ�
LPTSTR ResolvePrintId(CString PrintId)
{
	CString strReturn = _T("");
	LPTSTR strReturn1 = _T("");
	int iBufSize = PrintId.GetLength();
	//����MDL�ַ���
	int iPos = PrintId.Find(_T("MDL:"));
	if (iPos != -1)
	{
		//����MDL����ĵ�һ��;��
		int iPos1 = PrintId.Find(_T(";"), iPos);
		if ((iPos1 != -1) && (iPos1 > iPos) && (iPos1 <= iBufSize))
		{
			int iCount = iPos1 - iPos - 4;//-4��Ϊ������MDL:
			if (iCount > 0)
			{
				strReturn = PrintId.Mid(iPos + 4, iCount);  //+4��Ϊ������MDL:
				strReturn.TrimLeft();//�޳��ո�
				strReturn1 = strReturn.AllocSysString();
			}
		}
	}
	return strReturn1;
}
//��USB��ȡ������ID���������淽ʽ
LPTSTR ResolvePrintCommand(CString PrintId, CString print)
{
	CString strReturn = _T("");
	LPTSTR strReturn1 = _T("");
	int iBufSize = PrintId.GetLength();
	if (iBufSize > 0)
	{
		//����MDL�ַ���
		int iPos = PrintId.Find(_T("CMD:"));
		if (iPos != -1)
		{
			//����MDL����ĵ�һ��;��
			int iPos1 = PrintId.Find(_T(";"), iPos);
			if ((iPos1 != -1) && (iPos1 > iPos) && (iPos1 <= iBufSize))
			{
				int iCount = iPos1 - iPos - 4;//-4��Ϊ������CMD:
				if (iCount > 0)
				{
					strReturn = PrintId.Mid(iPos + 4, iCount);  //+4��Ϊ������MDL:
					strReturn.TrimLeft();//�޳��ո�
					strReturn1 = strReturn.AllocSysString();
				}
			}
		}
	}
	return strReturn1;
}
//��USB��ȡ������ID��������������
LPTSTR ResolvePrintProvider(CString Provider)
{
	CString strReturn = _T("");
	LPTSTR strReturn1 = _T("");
	int iBufSize = Provider.GetLength();
	//����MDL�ַ���
	int iPos = Provider.Find(_T("MFG:"));
	if (iPos != -1)
	{
		//����MDL����ĵ�һ��;��
		int iPos1 = Provider.Find(_T(";"), iPos);
		if ((iPos1 != -1) && (iPos1 > iPos) && (iPos1 <= iBufSize))
		{
			int iCount = iPos1 - iPos - 4;//-4��Ϊ������MFG:
			if (iCount > 0)
			{
				strReturn = Provider.Mid(iPos + 4, iCount);  //+4��Ϊ������MFG:
				strReturn.TrimLeft();//�޳��ո�
				LPTSTR strReturn1 = _T("");
				strReturn1 = strReturn.AllocSysString();
			}
		}
	}
	return strReturn1;
}