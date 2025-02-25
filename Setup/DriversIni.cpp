#include "stdafx.h"
#include "DriversIni.h"
#include "StrTranslate.h"
#include "DebugStuff.h"

extern CString g_MesTile;
extern CString g_Name;
extern UINT  g_uShowDlg;
extern CString g_MFG;
extern UINT   g_bOnboardDriver;

CDriversIni::CDriversIni()
{
	//获取当前程序路径
	CString strAppName;
	GetModuleFileName(NULL, strAppName.GetBuffer(_MAX_PATH), _MAX_PATH);
	//在其后面加上ini文件名
	strAppName.ReleaseBuffer();

	int nPos = strAppName.ReverseFind('\\');
	strAppName = strAppName.Left(nPos + 1);
	strAppName += _T("PrinterInfo.ini");  //加上ini文件名
	m_strFilePath = strAppName;
}

CDriversIni::~CDriversIni()
{
}

// 查询ini表文件是否存在，存在返回TRUE,否则FALSE
BOOL CDriversIni::GetFile()
{
	// 使用GetFileAttributes函数检查文件是否存在
	DWORD dwAttrib = GetFileAttributes(m_strFilePath);

	if ((dwAttrib != INVALID_FILE_ATTRIBUTES) &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))   //  && (dwAttrib & FILE_ATTRIBUTE_HIDDEN)
	{
		// 文件存在且不是一个目录
		return TRUE;
	}
	else
	{
		// 文件不存在或获取属性失败，记录日志
		WriteLogFile(_T("文件不存在或获取属性失败！"), TRUE);
		return FALSE;
	}
}

//获取所有系列名字
int CDriversIni::GetAllSeries(Serives* name)
{
	//获取系列个数
	int iCounts = 0;
	iCounts = GetPrivateProfileInt(_T("PRINTER_SERIES"), _T("Counts"), 0, m_strFilePath);

	//获取所有序列名字
	DWORD dwBufsize = 128;   //键值的大小
	DWORD dwReturn = 0;	   //实际返回的字符串的长度
	TCHAR* lpReturnstring = new TCHAR[dwBufsize];  //接收缓冲区
	ZeroMemory(lpReturnstring, dwBufsize + 1);  //将内存清0

	CString itostr;  //用来将int临时转为str类型

	//循环获取打印机系列名字
	for (int i = 0; i < iCounts; i++)
	{
		itostr.Format(_T("%d"), i);
		dwReturn = GetPrivateProfileStringW(_T("PRINTER_SERIES"), itostr, NULL, lpReturnstring, dwBufsize, m_strFilePath);
		//赋值系列名字
		name[i].SeriesName = lpReturnstring;
		ZeroMemory(lpReturnstring, dwReturn);  //将内存清0
	}

	delete[]lpReturnstring;
	lpReturnstring = NULL;
	return iCounts;
}

//获取该系列的所有打印机名字
int CDriversIni::GetAllPrintModel(CString  seriver, PrinterDefInfo* name)
{
	//获取型号个数
	int iCounts = 0;
	iCounts = GetPrivateProfileInt(seriver, _T("Counts"), 0, m_strFilePath);

	//获取所有序列名字
	DWORD dwBufsize = 128;   //键值的大小
	DWORD dwReturn = 0;	   //实际返回的字符串的长度
	TCHAR* lpReturnstring = new TCHAR[dwBufsize];  //接收缓冲区
	ZeroMemory(lpReturnstring, dwBufsize + 1);  //将内存清0

	CString itostr;  //用来将int临时转为str类型

	//循环获取打印机型号名字
	for (int i = 0; i < iCounts; i++)
	{
		itostr.Format(_T("%d"), i);
		dwReturn = GetPrivateProfileStringW(seriver, itostr, NULL, lpReturnstring, dwBufsize, m_strFilePath);
		//调用函数分离驱动名称和GPD
		GetGPDandPrintModel(lpReturnstring, &name[i]);
		GetPrintInstallFile(&name[i]);
		ZeroMemory(lpReturnstring, dwReturn);  //将内存清0
	}

	delete[]lpReturnstring;
	lpReturnstring = NULL;
	return iCounts;
}

//将键值分离为驱动名称和对应的GPD
void CDriversIni::GetGPDandPrintModel(TCHAR* lpReturnstring, PrinterDefInfo* Dirver)
{
	CString strReturn = lpReturnstring;

	int ilen = strReturn.GetLength();
	int iPosPre = 0;
	int iPos = 0;
	int iTime = 0;
	for (; iPos < ilen;)
	{
		iPos = strReturn.Find(_T(":"), iPosPre);
		if (iPos != -1)
		{
			switch (iTime)
			{
			case 0:
			{
				Dirver->InstallName = strReturn.Mid(iPosPre, iPos - iPosPre);
				iTime++;
				break;
			}
			case 1:
			{
				Dirver->PrintModel = strReturn.Mid(iPosPre, iPos - iPosPre);
				iTime++;
				break;
			}
			case 2:
			{
				Dirver->DefGPD = strReturn.Mid(iPosPre, iPos - iPosPre);
				iTime++;
				break;
			}
			case 3:
			{
				/*  CString point = strReturn.Mid(iPosPre, iPos - iPosPre);
				  Dirver->Point = _ttoi(point);*/
				Dirver->Point = strReturn.Mid(iPosPre, iPos - iPosPre);
				iTime++;
				break;
			}
			case 4:
			{
				/*  CString point = strReturn.Mid(iPosPre, iPos - iPosPre);
				Dirver->Point = _ttoi(point);*/
				Dirver->infName = strReturn.Mid(iPosPre, iPos - iPosPre);
				iTime++;
				break;
			}
			default:
				break;
			}
			iPosPre = iPos + 1;
		}
		else
		{
			//Dirver->DefCmd = strReturn.Mid(iPosPre, ilen - iPosPre);
			Dirver->InstallerSort = strReturn.Mid(iPosPre, ilen - iPosPre);
			break;
		}
	}
}

bool CDriversIni::GetPrintInstallFile(PrinterDefInfo* Dirver)
{
	bool ret = false;

	DWORD dwBufsize = 256;   //键值的大小
	DWORD dwReturn = 0;	   //实际返回的字符串的长度
	TCHAR* lpReturnstring = new TCHAR[dwBufsize];  //接收缓冲区
	ZeroMemory(lpReturnstring, dwBufsize + 1);  //将内存清0

	CString itostr;  //用来将int临时转为str类型

	//循环获取打印机型号名字
	// 改为以GPD文件名设定为section名，跳转到字节位置并获取相应字符串资源  add by hl 2019.12.10
	// 改为以类型区分字段为section名，跳转到字节位置并获取相应字符串资源  add by hl 2022.7.10
	dwReturn = GetPrivateProfileStringW(Dirver->InstallerSort, IDS_DependentFiles, NULL, lpReturnstring, dwBufsize, m_strFilePath);
	//dwReturn = GetPrivateProfileStringW(Dirver->DefGPD, IDS_DependentFiles, NULL, lpReturnstring, dwBufsize, m_strFilePath);
	if (dwReturn == 0)
	{
		delete[]lpReturnstring;
		lpReturnstring = NULL;
		return ret;
	}
	else
	{
		Dirver->DependentFiles = lpReturnstring;
		ZeroMemory(lpReturnstring, dwBufsize + 1);  //将内存清0
	}

	dwReturn = GetPrivateProfileStringW(Dirver->InstallerSort, ID_SSYSFile, NULL, lpReturnstring, dwBufsize, m_strFilePath);
	//dwReturn = GetPrivateProfileStringW(Dirver->DefGPD, ID_SSYSFile, NULL, lpReturnstring, dwBufsize, m_strFilePath);
	if (dwReturn == 0)
	{
		delete[]lpReturnstring;
		lpReturnstring = NULL;
		return ret;
	}
	else
	{
		Dirver->SYSFile = lpReturnstring;
		ZeroMemory(lpReturnstring, dwBufsize + 1);  //将内存清0
	}

	dwReturn = GetPrivateProfileStringW(Dirver->InstallerSort, IDS_CoverSYSFile, NULL, lpReturnstring, dwBufsize, m_strFilePath);
	//dwReturn = GetPrivateProfileStringW(Dirver->DefGPD, IDS_CoverSYSFile, NULL, lpReturnstring, dwBufsize, m_strFilePath);
	if (dwReturn == 0)
	{
		delete[]lpReturnstring;
		lpReturnstring = NULL;
		return ret;
	}
	else
	{
		Dirver->CoverSYSFile = lpReturnstring;
		ZeroMemory(lpReturnstring, dwBufsize + 1);  //将内存清0
	}

	//dwReturn = GetPrivateProfileStringW(Dirver->DefCmd, IDS_GPD, NULL, lpReturnstring, dwBufsize, m_strFilePath);
	//Dirver->DefGPD = lpReturnstring;
	//ZeroMemory(lpReturnstring, dwBufsize + 1);  //将内存清0

	delete[]lpReturnstring;
	lpReturnstring = NULL;

	return true;
}

void CDriversIni::GetSetting()
{
	DWORD dwBufsize = 256;   //键值的大小
	DWORD dwReturn = 0;	   //实际返回的字符串的长度
	TCHAR* lpReturnstring = new TCHAR[dwBufsize];  //接收缓冲区
	ZeroMemory(lpReturnstring, dwBufsize + 1);  //将内存清0

	CString itostr;  //用来将int临时转为str类型

	//循环获取打印机型号名字
	dwReturn = GetPrivateProfileStringW(IDS_Setting, IDS_MesTile, NULL, lpReturnstring, dwBufsize, m_strFilePath);
	g_MesTile = lpReturnstring;
	ZeroMemory(lpReturnstring, dwBufsize + 1);  //将内存清0

	dwReturn = GetPrivateProfileStringW(IDS_Setting, IDS_APPNAME, NULL, lpReturnstring, dwBufsize, m_strFilePath);
	g_Name = lpReturnstring;
	ZeroMemory(lpReturnstring, dwBufsize + 1);  //将内存清0

	// 获取MFG字段内容
	dwReturn = GetPrivateProfileStringW(IDS_Setting, IDS_MFG, NULL, lpReturnstring, dwBufsize, m_strFilePath);
	g_MFG = lpReturnstring;
	ZeroMemory(lpReturnstring, dwBufsize + 1);  //将内存清0

	g_uShowDlg = GetPrivateProfileInt(IDS_Setting, IDS_SHOWDLG, 1, m_strFilePath);

	g_bOnboardDriver = GetPrivateProfileInt(IDS_Setting, IDS_OnboardDriver, 0, m_strFilePath);

	delete[]lpReturnstring;
	lpReturnstring = NULL;

	return;
}