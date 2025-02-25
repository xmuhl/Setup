// InfFileHelper.h: interface for the CInfFileHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INFFILEHELPER_H__25AEEEDE_7CC9_406E_BCDE_BC0744845379__INCLUDED_)
#define AFX_INFFILEHELPER_H__25AEEEDE_7CC9_406E_BCDE_BC0744845379__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CInfFileHelper
{
public:
	static BOOL MatchHwID(const CStringArray& szArrHwID, CString szHwID);
	BOOL GetHardwareID(CString szModel, CStringArray& szArrHwID);
	BOOL GetManufacturer(CStringArray& szManufacturer);
	BOOL OpenInfFile(CString szInfFile);
	CInfFileHelper();
	virtual ~CInfFileHelper();
public:
	BOOL SeekLine(CString szLine, UINT uForm = CFile::begin);
	CStdioFile m_file;
};

#endif // !defined(AFX_INFFILEHELPER_H__25AEEEDE_7CC9_406E_BCDE_BC0744845379__INCLUDED_)
