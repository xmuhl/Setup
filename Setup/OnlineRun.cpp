#include "stdafx.h"
#include "OnlineRun.h"
#include "GetSysVersion.h"
#include "InfFileHelper.h"
#include "DebugStuff.h"
#include <cfgmgr32.h>

#include "DriverSetupHelper.h"
#include "UDefault.h"

OnlineRun::OnlineRun()
{
	m_hDevInfo = NULL;
	m_nDeviceIndex = -1;
}

OnlineRun::~OnlineRun()
{
}

// ��ȡinf·��
CString OnlineRun::GetInfPath(LPCTSTR PointMark, BOOL OsX64, LPCTSTR infName)
{
	CString strReturn = _T("");

	//�޸�ʹ��GetModuleFileName��ȡ��ǰ·������ֹ·���ı�ʱ�����ֻ�ȡ���������
	CString strAppName;
	GetModuleFileName(NULL, strAppName.GetBuffer(_MAX_PATH), _MAX_PATH);
	//����������ini�ļ���
	strAppName.ReleaseBuffer();
	int nPos = strAppName.ReverseFind('\\');
	strAppName = strAppName.Left(nPos + 1);

	TCHAR* byCurPath = new TCHAR[_MAX_PATH];
	ZeroMemory(byCurPath, _MAX_PATH);
	lstrcpy(byCurPath, strAppName);

	DebugMesBox(byCurPath);

	// ����inf�ļ���ϵͳ·����
	TCHAR OEMSourceMediaLocation[BUFSIZE] = { 0 }; //���inf��Ŀ¼·��
	lstrcpy(OEMSourceMediaLocation, byCurPath);

	TCHAR InfPath[BUFSIZE] = { 0 };  //���inf����·��

	lstrcat(OEMSourceMediaLocation, PointMark);
	lstrcat(OEMSourceMediaLocation, _T("\\"));
	lstrcpy(InfPath, OEMSourceMediaLocation);
	lstrcat(InfPath, infName);
	strReturn = InfPath;

	return strReturn;
}

// SetupDiGetINFClass ����ָ������
typedef BOOL(WINAPI* PFN_SetupDiGetINFClassW)(
	PCWSTR InfName,
	GUID* ClassGuid,
	PWSTR ClassName,
	DWORD ClassNameSize,
	PDWORD RequiredSize
	);

// SetupDiGetClassDevs ����ָ������
typedef HDEVINFO(WINAPI* PFN_SetupDiGetClassDevsW)(
	const GUID* ClassGuid,
	PCWSTR Enumerator,
	HWND hwndParent,
	DWORD Flags
	);

// SetupDiGetDeviceInstallParams ����ָ������
typedef BOOL(WINAPI* PFN_SetupDiGetDeviceInstallParamsW)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	PSP_DEVINSTALL_PARAMS_W DeviceInstallParams
	);

// SetupDiSetDeviceInstallParams ����ָ������
typedef BOOL(WINAPI* PFN_SetupDiSetDeviceInstallParamsW)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	PSP_DEVINSTALL_PARAMS_W DeviceInstallParams
	);

// SetupDiBuildDriverInfoList ����ָ������
typedef BOOL(WINAPI* PFN_SetupDiBuildDriverInfoList)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	DWORD DriverType
	);

// SetupDiEnumDriverInfo ����ָ������
typedef BOOL(WINAPI* PFN_SetupDiEnumDriverInfoW)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	DWORD DriverType,
	DWORD MemberIndex,
	PSP_DRVINFO_DATA_W DriverInfoData
	);

// SetupDiGetDriverInfoDetail ����ָ�����ͣ�������ģ�
typedef BOOL(WINAPI* PFN_SetupDiGetDriverInfoDetailW)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	PSP_DRVINFO_DATA_W DriverInfoData,
	PSP_DRVINFO_DETAIL_DATA_W DriverInfoDetailData,
	DWORD DriverInfoDetailDataSize,
	PDWORD RequiredSize
	);

// SetupDiDestroyDriverInfoList ����ָ������
typedef BOOL(WINAPI* PFN_SetupDiDestroyDriverInfoList)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	DWORD DriverType
	);

// SetupDiEnumDeviceInfo ����ָ������
typedef BOOL(WINAPI* PFN_SetupDiEnumDeviceInfo)(
	HDEVINFO DeviceInfoSet,
	DWORD MemberIndex,
	PSP_DEVINFO_DATA DeviceInfoData
	);

// SetupDiGetDeviceRegistryProperty ����ָ������
typedef BOOL(WINAPI* PFN_SetupDiGetDeviceRegistryPropertyW)(
	HDEVINFO DeviceInfoSet,
	PSP_DEVINFO_DATA DeviceInfoData,
	DWORD Property,
	PDWORD PropertyRegDataType,
	PBYTE PropertyBuffer,
	DWORD PropertyBufferSize,
	PDWORD RequiredSize
	);

// SetupDiGetClassImageIndex ����ָ������
typedef BOOL(WINAPI* PFN_SetupDiGetClassImageIndex)(
	const SP_CLASSIMAGELIST_DATA* ClassImageListData,
	const GUID* ClassGuid,
	int* ImageIndex
	);

// SetupDiDestroyDeviceInfoList ����ָ������
typedef BOOL(WINAPI* PFN_SetupDiDestroyDeviceInfoList)(
	HDEVINFO DeviceInfoSet
	);

// �޶���� OpenInf ����
BOOL OnlineRun::OpenInf(CString szInfFile)
{
	Reset(); // ���õ�ǰ״̬

	DebugMesBox(szInfFile); // ������Ϣ

	CStringArray szArrHwID; // �洢Ӳ��ID����

	CInfFileHelper infHelper;

	if (!infHelper.OpenInfFile(szInfFile))
	{
		return FALSE; // ��INF�ļ�ʧ��
	}

	CStringArray szArr;

	infHelper.GetManufacturer(szArr); // ��ȡӲ��������

	for (int i = 0; i < szArr.GetSize(); i++)
	{
		CStringArray szArrHwIDTemp; // ��ʱ�洢Ӳ��ID
		infHelper.GetHardwareID(szArr[i], szArrHwIDTemp);
		szArrHwID.Append(szArrHwIDTemp); // �ϲ�Ӳ��ID
	}

	m_szInfFile = szInfFile; // ����INF�ļ�·��

	// ��̬���� SetupAPI.dll
	HMODULE hSetupAPI = LoadLibrary(TEXT("SetupAPI.dll"));
	if (hSetupAPI == NULL)
	{
		GetErrAndLog(_T("�޷����� SetupAPI.dll"));
		return FALSE;
	}

	// ��ȡ������Ҫ�ĺ�����ַ��������׺
	PFN_SetupDiGetINFClassW pSetupDiGetINFClass =
		(PFN_SetupDiGetINFClassW)GetProcAddress(hSetupAPI, "SetupDiGetINFClassW");
	PFN_SetupDiGetClassDevsW pSetupDiGetClassDevs =
		(PFN_SetupDiGetClassDevsW)GetProcAddress(hSetupAPI, "SetupDiGetClassDevsW");
	PFN_SetupDiGetDeviceInstallParamsW pSetupDiGetDeviceInstallParams =
		(PFN_SetupDiGetDeviceInstallParamsW)GetProcAddress(hSetupAPI, "SetupDiGetDeviceInstallParamsW");
	PFN_SetupDiSetDeviceInstallParamsW pSetupDiSetDeviceInstallParams =
		(PFN_SetupDiSetDeviceInstallParamsW)GetProcAddress(hSetupAPI, "SetupDiSetDeviceInstallParamsW");
	PFN_SetupDiBuildDriverInfoList pSetupDiBuildDriverInfoList =
		(PFN_SetupDiBuildDriverInfoList)GetProcAddress(hSetupAPI, "SetupDiBuildDriverInfoList");
	PFN_SetupDiEnumDriverInfoW pSetupDiEnumDriverInfo =
		(PFN_SetupDiEnumDriverInfoW)GetProcAddress(hSetupAPI, "SetupDiEnumDriverInfoW");
	PFN_SetupDiGetDriverInfoDetailW pSetupDiGetDriverInfoDetail =
		(PFN_SetupDiGetDriverInfoDetailW)GetProcAddress(hSetupAPI, "SetupDiGetDriverInfoDetailW");
	PFN_SetupDiDestroyDriverInfoList pSetupDiDestroyDriverInfoList =
		(PFN_SetupDiDestroyDriverInfoList)GetProcAddress(hSetupAPI, "SetupDiDestroyDriverInfoList");
	PFN_SetupDiEnumDeviceInfo pSetupDiEnumDeviceInfo =
		(PFN_SetupDiEnumDeviceInfo)GetProcAddress(hSetupAPI, "SetupDiEnumDeviceInfo");
	PFN_SetupDiGetDeviceRegistryPropertyW pSetupDiGetDeviceRegistryProperty =
		(PFN_SetupDiGetDeviceRegistryPropertyW)GetProcAddress(hSetupAPI, "SetupDiGetDeviceRegistryPropertyW");
	PFN_SetupDiGetClassImageIndex pSetupDiGetClassImageIndex =
		(PFN_SetupDiGetClassImageIndex)GetProcAddress(hSetupAPI, "SetupDiGetClassImageIndex");
	PFN_SetupDiDestroyDeviceInfoList pSetupDiDestroyDeviceInfoListPtr =
		(PFN_SetupDiDestroyDeviceInfoList)GetProcAddress(hSetupAPI, "SetupDiDestroyDeviceInfoList");

	// ������к����Ƿ�ɹ���ȡ
	if (!pSetupDiGetINFClass || !pSetupDiGetClassDevs || !pSetupDiGetDeviceInstallParams ||
		!pSetupDiSetDeviceInstallParams || !pSetupDiBuildDriverInfoList || !pSetupDiEnumDriverInfo ||
		!pSetupDiGetDriverInfoDetail || !pSetupDiDestroyDriverInfoList ||
		!pSetupDiEnumDeviceInfo || !pSetupDiGetDeviceRegistryProperty || !pSetupDiGetClassImageIndex ||
		!pSetupDiDestroyDeviceInfoListPtr)
	{
		GetErrAndLog(_T("�޷���ȡ SetupAPI ����ĺ�����ַ"));
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	BOOL bOK = FALSE;
	GUID guid;
	TCHAR szClassName[MAX_PATH] = { 0 };

	// ��ȡINF�ļ�����GUID������
	bOK = pSetupDiGetINFClass(szInfFile, &guid, szClassName, MAX_PATH, NULL);
	if (!bOK)
	{
		DumpErrorMes(_T("SetupDiGetINFClass"));
		pSetupDiDestroyDeviceInfoListPtr(m_hDevInfo); // ʹ�ö�̬��������
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	// ��ȡ�豸��Ϣ����
	m_hDevInfo = pSetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_PROFILE);
	if (m_hDevInfo == INVALID_HANDLE_VALUE)
	{
		DumpErrorMes(_T("SetupDiGetClassDevs"));
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	// ��ȡ�豸��װ����
	SP_DEVINSTALL_PARAMS_W sppd = { 0 };
	sppd.cbSize = sizeof(sppd);
	bOK = pSetupDiGetDeviceInstallParams(m_hDevInfo, NULL, &sppd);
	if (!bOK)
	{
		DumpErrorMes(_T("SetupDiGetDeviceInstallParams"));
		pSetupDiDestroyDeviceInfoListPtr(m_hDevInfo);
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	// �����豸��װ����
	sppd.Flags = DI_ENUMSINGLEINF; // ֻö�ٵ���INF
	sppd.FlagsEx = DI_FLAGSEX_ALLOWEXCLUDEDDRVS; // �����ų���������
	wcscpy_s(sppd.DriverPath, szInfFile);
	bOK = pSetupDiSetDeviceInstallParams(m_hDevInfo, NULL, &sppd);
	if (!bOK)
	{
		DumpErrorMes(_T("SetupDiSetDeviceInstallParams"));
		pSetupDiDestroyDeviceInfoListPtr(m_hDevInfo);
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	// ��������������Ϣ�б�
	bOK = pSetupDiBuildDriverInfoList(m_hDevInfo, NULL, SPDIT_CLASSDRIVER);
	if (!bOK)
	{
		DumpErrorMes(_T("SetupDiBuildDriverInfoList"));
		pSetupDiDestroyDeviceInfoListPtr(m_hDevInfo);
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	// ö������������Ϣ
	int nIndex = 0;
	SP_DRVINFO_DATA_W driverInfo = { 0 };
	driverInfo.cbSize = sizeof(driverInfo);
	TCHAR szHardID[MAX_PATH + 1] = { 0 };

	do
	{
		bOK = pSetupDiEnumDriverInfo(m_hDevInfo, NULL, SPDIT_CLASSDRIVER, nIndex++, &driverInfo);
		if (bOK)
		{
			// �����㹻�Ļ�����
			DWORD dwRequiredSize = 0;
			pSetupDiGetDriverInfoDetail(m_hDevInfo, NULL, &driverInfo, NULL, 0, &dwRequiredSize);

			PSP_DRVINFO_DETAIL_DATA_W pDriverDetail = (PSP_DRVINFO_DETAIL_DATA_W)new BYTE[dwRequiredSize];
			pDriverDetail->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA_W);

			bOK = pSetupDiGetDriverInfoDetail(
				m_hDevInfo,
				NULL,               // DeviceInfoData������Ϊ NULL
				&driverInfo,        // DriverInfoData
				pDriverDetail,      // DriverInfoDetailData
				dwRequiredSize,     // DriverInfoDetailDataSize
				NULL                // RequiredSize
			);
			if (bOK)
			{
				wcsncpy_s(szHardID, pDriverDetail->HardwareID, MAX_PATH);
			}

			delete[] pDriverDetail; // �ͷŷ�����ڴ�
		}
	} while (bOK);

	// ��������������Ϣ�б�
	pSetupDiDestroyDriverInfoList(m_hDevInfo, NULL, SPDIT_CLASSDRIVER);

	// ö���豸��Ϣ
	nIndex = -1;
	do
	{
		SP_DEVINFO_DATA spdd = { 0 };
		spdd.cbSize = sizeof(spdd);
		bOK = pSetupDiEnumDeviceInfo(m_hDevInfo, ++nIndex, &spdd);
		if (bOK)
		{
			TCHAR szBuf[2000] = { 0 };
			// ��ȡӲ��ID����
			pSetupDiGetDeviceRegistryProperty(m_hDevInfo, &spdd, SPDRP_HARDWAREID, NULL, (PBYTE)szBuf, sizeof(szBuf), NULL);

			// �ж�Ӳ��ID�Ƿ�ƥ��
			if (CInfFileHelper::MatchHwID(szArrHwID, szBuf))
			{
				m_szCurHwID = szBuf; // ���浱ǰӲ��ID

				// ��ȡ�豸����
				pSetupDiGetDeviceRegistryProperty(m_hDevInfo, &spdd, SPDRP_DEVICEDESC, NULL, (PBYTE)szBuf, sizeof(szBuf), NULL);

				int nImageIndex = 0;
				m_nDeviceIndex = nIndex;

				// ��ȡ��ͼ������
				pSetupDiGetClassImageIndex(&m_sid, &spdd.ClassGuid, &nImageIndex);
			}
		}
	} while (bOK);

	// ����Ƿ��ҵ�ƥ���Ӳ��ID
	if (m_szCurHwID.IsEmpty())
	{
		GetErrAndLog(_T("δ�ҵ�����ϵ��豸ID"));
		pSetupDiDestroyDeviceInfoListPtr(m_hDevInfo);
		FreeLibrary(hSetupAPI);
		return FALSE;
	}

	// �ͷ� SetupAPI.dll
	pSetupDiDestroyDeviceInfoListPtr(m_hDevInfo);
	FreeLibrary(hSetupAPI);

	return TRUE; // �ɹ�
}

// �޶���� Reset ����
//void OnlineRun::Reset()
//{
//	if (m_hDevInfo)
//	{
//		// ��̬���� SetupAPI.dll
//		HMODULE hSetupAPI = LoadLibrary(TEXT("SetupAPI.dll"));
//		if (hSetupAPI)
//		{
//			// ���庯��ָ������
//			typedef BOOL(WINAPI* PFN_SetupDiDestroyDeviceInfoList)(
//				HDEVINFO DeviceInfoSet
//				);
//			// ��ȡ������ַ
//			PFN_SetupDiDestroyDeviceInfoList pSetupDiDestroyDeviceInfoList =
//				(PFN_SetupDiDestroyDeviceInfoList)GetProcAddress(hSetupAPI, "SetupDiDestroyDeviceInfoList");
//
//			if (pSetupDiDestroyDeviceInfoList)
//			{
//				// ���ú��������豸��Ϣ�б�
//				pSetupDiDestroyDeviceInfoList(m_hDevInfo);
//			}
//			else
//			{
//				// ��ȡ������ַʧ�ܣ��������
//				GetErrAndLog(_T("�޷���ȡ���� SetupDiDestroyDeviceInfoList �ĵ�ַ"));
//			}
//			// �ͷŶ�̬��
//			FreeLibrary(hSetupAPI);
//		}
//		else
//		{
//			// ���ؿ�ʧ�ܣ��������
//			GetErrAndLog(_T("�޷����� SetupAPI.dll"));
//		}
//		m_hDevInfo = NULL;
//	}
//	m_szCurHwID.Empty();
//	m_szInfFile.Empty();
//}

void OnlineRun::Reset()
{
	if (m_hDevInfo)
	{
		// ��̬���� SetupAPI.dll
		HMODULE hSetupAPI = LoadLibrary(TEXT("SetupAPI.dll"));
		if (hSetupAPI)
		{
			// ���庯��ָ������
			typedef BOOL(WINAPI* PFN_SetupDiDestroyDeviceInfoList)(
				HDEVINFO DeviceInfoSet
				);
			// ��ȡ������ַ
			PFN_SetupDiDestroyDeviceInfoList pSetupDiDestroyDeviceInfoList =
				(PFN_SetupDiDestroyDeviceInfoList)GetProcAddress(hSetupAPI, "SetupDiDestroyDeviceInfoList");

			if (pSetupDiDestroyDeviceInfoList)
			{
				// ���ú��������豸��Ϣ�б�
				pSetupDiDestroyDeviceInfoList(m_hDevInfo);
			}
			else
			{
				// ��ȡ������ַʧ�ܣ��������
				GetErrAndLog(_T("�޷���ȡ���� SetupDiDestroyDeviceInfoList �ĵ�ַ"));
			}
			// �ͷŶ�̬��
			FreeLibrary(hSetupAPI);
		}
		else
		{
			// ���ؿ�ʧ�ܣ��������
			GetErrAndLog(_T("�޷����� SetupAPI.dll"));
		}
		m_hDevInfo = NULL;
	}
	m_szCurHwID.Empty();
	m_szInfFile.Empty();

	// ���ɨ����Ӳ���Ķ��Ĵ���
	// ��̬���� cfgmgr32.dll
	HMODULE hCfgMgr32 = LoadLibrary(TEXT("cfgmgr32.dll"));
	if (hCfgMgr32)
	{
		// ���庯��ָ������
		typedef CONFIGRET(WINAPI* PFN_CM_Locate_DevNodeW)(
			PDEVINST pdnDevInst,
			DEVINSTID_W pDeviceID,
			ULONG ulFlags
			);
		typedef CONFIGRET(WINAPI* PFN_CM_Reenumerate_DevNode)(
			DEVINST dnDevInst,
			ULONG ulFlags
			);

		// ��ȡ������ַ
		PFN_CM_Locate_DevNodeW pCM_Locate_DevNodeW =
			(PFN_CM_Locate_DevNodeW)GetProcAddress(hCfgMgr32, "CM_Locate_DevNodeW");
		PFN_CM_Reenumerate_DevNode pCM_Reenumerate_DevNode =
			(PFN_CM_Reenumerate_DevNode)GetProcAddress(hCfgMgr32, "CM_Reenumerate_DevNode");

		if (pCM_Locate_DevNodeW && pCM_Reenumerate_DevNode)
		{
			DEVINST devRoot = 0;
			CONFIGRET cr = pCM_Locate_DevNodeW(&devRoot, NULL, CM_LOCATE_DEVNODE_NORMAL);
			if (cr == CR_SUCCESS)
			{
				// ����Ӳ������ö��
				cr = pCM_Reenumerate_DevNode(devRoot, CM_REENUMERATE_SYNCHRONOUS);
				if (cr != CR_SUCCESS)
				{
					// �������
					CString strError;
					strError.Format(_T("CM_Reenumerate_DevNode ִ��ʧ�ܣ������룺%lu"), cr);
					GetErrAndLog(strError);
				}
			}
			else
			{
				// �������
				CString strError;
				strError.Format(_T("CM_Locate_DevNodeW ִ��ʧ�ܣ������룺%lu"), cr);
				GetErrAndLog(strError);
			}
		}
		else
		{
			// ��ȡ������ַʧ�ܣ��������
			GetErrAndLog(_T("�޷���ȡ���� CM_Locate_DevNodeW �� CM_Reenumerate_DevNode �ĵ�ַ"));
		}

		// �ͷŶ�̬��
		FreeLibrary(hCfgMgr32);
	}
	else
	{
		// ���ؿ�ʧ�ܣ��������
		GetErrAndLog(_T("�޷����� cfgmgr32.dll"));
	}
}

//// ��inf�ļ�
//BOOL OnlineRun::OpenInf(CString szInfFile)
//{
//	Reset();
//	DebugMesBox(szInfFile);
//	CStringArray szArrHwID;
//
//	CInfFileHelper infHelper;
//	if (!infHelper.OpenInfFile(szInfFile))
//	{
//		return FALSE;
//	}
//
//	CStringArray szArr;
//	infHelper.GetManufacturer(szArr);					//Get Hardware Manufacture
//	for (int i = 0; i < szArr.GetSize(); i++)
//	{
//		CStringArray szArrHwIDTemp;						 //Get HdID
//		infHelper.GetHardwareID(szArr[i], szArrHwIDTemp);
//		szArrHwID.Append(szArrHwIDTemp);
//	}
//
//	m_szInfFile = szInfFile;
//
//	BOOL bOK = FALSE;
//	GUID guid;
//	TCHAR szClassName[MAX_PATH];
//	bOK = SetupDiGetINFClass(szInfFile, &guid, szClassName, MAX_PATH, 0);    //��ȡ�˿����ƺ�GUID
//	if (!bOK)
//	{
//		DumpErrorMes(_T("SetupDiGetINFClass"));
//		return FALSE;
//	}
//
//	m_hDevInfo = SetupDiGetClassDevs(&guid, NULL, NULL,
//		DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_PROFILE);  //����ָ����USB�豸��GUID��ȡ�豸��Ϣ��
//
//	SP_DEVINSTALL_PARAMS sppd = { 0 };
//	sppd.cbSize = sizeof(sppd);
//	bOK = SetupDiGetDeviceInstallParams(m_hDevInfo, NULL, &sppd);  //�����豸��װ���������豸����Ϣ���ض��豸����ϢԪ��
//
//	sppd.Flags = DI_ENUMSINGLEINF;  //��־���Ա���ָ��SP_DEVINSTALL_PARAMS.DriverPathֻ����INFӦ������
//
//	sppd.FlagsEx = DI_FLAGSEX_ALLOWEXCLUDEDDRVS;
//	wcscpy_s(sppd.DriverPath, szInfFile);
//	bOK = SetupDiSetDeviceInstallParams(m_hDevInfo, NULL, &sppd);
//
//	bOK = SetupDiBuildDriverInfoList(m_hDevInfo, NULL, SPDIT_CLASSDRIVER); //����һ����һ���ض����豸������ȫ�򼶵��豸���������б��е���Ϣ��ص�����������б�
//
//	int nIndex = 0;
//	SP_DRVINFO_DATA driverInfo = { 0 };
//	driverInfo.cbSize = sizeof(driverInfo);
//	TCHAR szHardID[MAX_PATH + 1];
//	szHardID[0] = '\0';
//	do
//	{
//		bOK = SetupDiEnumDriverInfo(m_hDevInfo, 0, SPDIT_CLASSDRIVER, nIndex++, &driverInfo);//�о������������б�ĳ�Ա
//		if (bOK)
//		{
//			char szBuf[2000];
//			ZeroMemory(szBuf, 2000);
//			DWORD dwRequiredSize = 0;
//			PSP_DRVINFO_DETAIL_DATA pDriverDetail = (PSP_DRVINFO_DETAIL_DATA)szBuf;
//			pDriverDetail->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
//			if (SetupDiGetDriverInfoDetail(m_hDevInfo, NULL, &driverInfo,  //��������������Ϣϸ�������豸����Ϣ�����豸��Ϣ�����ض��豸����ϢԪ�ء�
//				pDriverDetail, 2000, &dwRequiredSize))
//			{
//				wcsncpy_s(szHardID, pDriverDetail->HardwareID, MAX_PATH);
//			}
//		}
//	} while (bOK);
//	SetupDiDestroyDriverInfoList(m_hDevInfo, NULL, SPDIT_CLASSDRIVER);  //ɾ�����������б�
//	nIndex = -1;
//
//	do
//	{
//		SP_DEVINFO_DATA spdd = { 0 };
//		spdd.cbSize = sizeof(spdd);
//		bOK = SetupDiEnumDeviceInfo(m_hDevInfo, ++nIndex, &spdd);   //ö��ָ���豸��Ϣ���ϵĳ�Ա
//		if (bOK)
//		{
//			TCHAR szBuf[2000];
//			SetupDiGetDeviceRegistryProperty(m_hDevInfo, &spdd, SPDRP_HARDWAREID, NULL, (PBYTE)szBuf, 2000, NULL);
//
//			if (CInfFileHelper::MatchHwID(szArrHwID, szBuf))
//			{
//				m_szCurHwID = szBuf;
//
//				SetupDiGetDeviceRegistryProperty(m_hDevInfo, &spdd, SPDRP_DEVICEDESC,
//					NULL, (PBYTE)szBuf, 2000, NULL);
//
//				int nImageIndex = 0;
//				m_nDeviceIndex = nIndex;
//				SetupDiGetClassImageIndex(&m_sid, &spdd.ClassGuid, &nImageIndex);
//			}
//		}
//	} while (bOK);
//
//	if (m_szCurHwID == _T(""))
//	{
//		GetErrAndLog(_T("δ�ҵ�����ϵ��豸ID"));
//		return FALSE;
//	}
//
//	return TRUE;
//}

// void OnlineRun::Reset()
// {
// 	if (m_hDevInfo)
// 	{
// 		SetupDiDestroyDeviceInfoList(m_hDevInfo);// ���ö�ٺ���豸��Ϣ�б�
// 		m_hDevInfo = NULL;
// 	}
//
// 	m_szCurHwID.Empty();
// 	m_szInfFile.Empty();
// }
//

BOOL OnlineRun::AddNewDevice()
{
	BOOL bRetrun = FALSE;

	CDriverSetupHelper ds;
	if (ds.Install(m_szInfFile, m_szCurHwID))
	{
		DebugMesBox(_T("��װ�ɹ�"));
		bRetrun = TRUE;
	}

	return bRetrun;
}