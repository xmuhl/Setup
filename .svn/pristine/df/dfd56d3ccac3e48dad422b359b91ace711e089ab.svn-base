// InfFileHelper.cpp: implementation of the CInfFileHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InfFileHelper.h"
#include "GetSysVersion.h"
#include "DebugStuff.h"

//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#define new DEBUG_NEW
//#endif
//
//#ifndef DT
//#define DT	DebugTrace
//extern BOOL DebugTrace(const char* lpszFormat, ...);
//#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInfFileHelper::CInfFileHelper()
{
}

CInfFileHelper::~CInfFileHelper()
{
}

BOOL CInfFileHelper::OpenInfFile(CString szInfFile)
{
	ASSERT(m_file.m_hFile == CFile::hFileNull);

	return m_file.Open(szInfFile, CFile::modeRead | CFile::shareDenyNone);
}

//得到[Manufacturer]段
//格式范例:
//[Manufacturer]
//%Philips%=Philips
//每段必须在同一行
BOOL CInfFileHelper::GetManufacturer(CStringArray& szArrManu)
{
	ASSERT(m_file.m_hFile != CFile::hFileNull);

	szArrManu.RemoveAll();

	BOOL bOK = SeekLine(_T("[Manufacturer]"));
	if (!bOK)
		return FALSE;

	CString szLine;
	CString szItem;
	while (m_file.ReadString(szLine))
	{
		szLine.TrimLeft();
		szLine.TrimRight();
		if (szLine.IsEmpty())
			continue;
		if (szLine[0] == ';')
			continue;

		if (szLine.Find('[') != -1)
		{
			szLine.TrimLeft();
			if (szLine[0] != ';')//is comment?
				return bOK;//reached the next section
			else
				continue;
		}
		int nSectionPos = szLine.Find('=');
		if (nSectionPos != -1)
		{
			szItem = szLine.Right(szLine.GetLength() - nSectionPos - 1);
			szItem.TrimLeft();
			szItem.TrimRight();
			//DT(szItem);

			szArrManu.Add(szItem);
		}
	}
	return bOK;
}

//得到指定szModel下的所有hardware ID
//格式范例:
//[Philips]
//%Cap7134.DeviceFM1216ME%=Cap7134.FM1216ME,PCI\VEN_1131&DEV_7134&SUBSYS_235c1131
//%Cap7134.DeviceFQ1216ME%=Cap7134.FQ1216ME,PCI\VEN_1131&DEV_7134&SUBSYS_235d1131
//每段必须在同一行

BOOL CInfFileHelper::GetHardwareID(CString szModel, CStringArray& szArrHwID)
{
	ASSERT(m_file.m_hFile != CFile::hFileNull);

	szArrHwID.RemoveAll();
	int  iLeng = szModel.GetLength();
	int iPrePos = 0;
	int iPos = 0;
	CStringArray ArraySection;
	do
	{
		CString	SectionName = _T("");
		iPos = szModel.Find(_T(","), iPrePos);
		if (iPos != -1)
		{
			SectionName = szModel.Mid(iPrePos, iPos - iPrePos);
			ArraySection.Add(SectionName);
			iPrePos = iPos + 1;
		}
		else
		{
			SectionName = szModel.Mid(iPrePos, iLeng - iPrePos);
			ArraySection.Add(SectionName);
			break;
		}
	} while (iPos < iLeng);

	BOOL bOK = false;
	if (ArraySection.GetSize() >= 2)
	{
		bOK = SeekLine(_T("[") + ArraySection.GetAt(0) + _T(".") + ArraySection.GetAt(1) + _T("]"));
	}
	else
	{
		bOK = SeekLine(_T("[") + ArraySection.GetAt(0) + _T("]"));
	}
	if (!bOK)
		return FALSE;

	CString szLine;
	CString szItem;
	while (m_file.ReadString(szLine))
	{
		szLine.TrimLeft();
		szLine.TrimRight();
		if (szLine.IsEmpty())
			continue;
		if (szLine[0] == ';')
			continue;

		if (szLine.Find('[') != -1)
		{
			szLine.TrimLeft();
			if (szLine[0] != ';')//is comment?
				return bOK;//reached the next section
			else
				continue;
		}
		//int nPos = szLine.Find('=');
		int nComma = szLine.Find(',');
		if (nComma != -1)//nPos != -1 &&
		{
			szItem = szLine.Right(szLine.GetLength() - nComma - 1);
			szItem.TrimLeft();
			szItem.TrimRight();
			//DT(szItem);

			szArrHwID.Add(szItem);
		}
	}
	return bOK;
}

BOOL CInfFileHelper::SeekLine(CString szLine, UINT uFrom)
{
	ASSERT(uFrom == CFile::begin || CFile::current);
	if (uFrom == CFile::begin)
		m_file.SeekToBegin();

	CString szThisLine;

	while (m_file.ReadString(szThisLine))
	{
		if (szThisLine.Find(szLine) != -1)
			return TRUE;
	}
	return FALSE;
}

BOOL CInfFileHelper::MatchHwID(const CStringArray& szArrHwID, CString szHwID)
{
	BOOL bRet = FALSE;

	int nc = (int)szArrHwID.GetSize();

	szHwID.MakeUpper();
	CString szArrItem;

	for (int i = 0; i < nc; i++)
	{
		szArrItem = szArrHwID[i];
		szArrItem.MakeUpper();
		//GetErrAndLog(szArrItem + _T("\r\n"));
		if (szHwID.Find(szArrItem) != -1)
		{
			GetErrAndLog(szArrItem);
			/*	int nLen = szArrItem.GetLength();
				if(szHwID.GetLength() > nLen)
				{
				if(szHwID[nLen] != '&'
				&& szHwID[nLen] != '-'
				&&szHwID[nLen] != '#'
				&&szHwID[nLen] != '_'
				)
				continue;
				}*/

				//return TRUE;
			bRet = TRUE;
			break;
		}
	}
	//return FALSE;
	return bRet;
}