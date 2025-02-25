/************************************************************************

FileName:	DebugStuff.h

Writer:	黄磊

create Date:	090629

Version:		1.0

Main Content: 调试信息自定义输出

rebuild:

/************************************************************************/
#pragma once

#ifdef _DEBUG

#define DebugMesPrint			// debug版本，开启自定义调试输出开关

#endif

#define  WriteLog					// 开启写日志开关

// 调试输出发现错误信息的程序块，错误代码及错误内容
void DumpErrorMes(LPCTSTR);

void DebugLogOutput(LPCTSTR lpszMessage);
// 调试输出用户自定义提示信息内容
void DebugMesBox(LPCTSTR);

//将日志信息写入txt
void WriteLogFile(CString strLog, BOOL bDump = FALSE);
CString GetErrorMsg(DWORD dwErrorCode);
void LogOutput(CString strMsg);
void GetErrAndLog(CString strError);