/************************************************************************

FileName:	DebugStuff.h

Writer:	����

create Date:	090629

Version:		1.0

Main Content: ������Ϣ�Զ������

rebuild:

/************************************************************************/
#pragma once

#ifdef _DEBUG

#define DebugMesPrint			// debug�汾�������Զ�������������

#endif

#define  WriteLog					// ����д��־����

// ����������ִ�����Ϣ�ĳ���飬������뼰��������
void DumpErrorMes(LPCTSTR);

void DebugLogOutput(LPCTSTR lpszMessage);
// ��������û��Զ�����ʾ��Ϣ����
void DebugMesBox(LPCTSTR);

//����־��Ϣд��txt
void WriteLogFile(CString strLog, BOOL bDump = FALSE);
CString GetErrorMsg(DWORD dwErrorCode);
void LogOutput(CString strMsg);
void GetErrAndLog(CString strError);