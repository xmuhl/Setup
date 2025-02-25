/************************************************************************

FileName:	DebugStuff.cpp

Writer:	黄磊

create Date:	090629

Version:	1.0

Main Content:
			调试输出发现错误信息的程序块，错误代码及错误内容
			void DumpErrorMes(LPCTSTR);

			调试输出用户自定义提示信息内容
			void DebugMesBox(LPCTSTR);

rebuild:

/************************************************************************/

//#include "pch.h"
#include "stdafx.h"
#include "DebugStuff.h"
#include "StrTranslate.h"
#include <Windows.h>
#include <WinBase.h>
#include <cstdio>
#include <afxwin.h>
#include <afxdlgs.h>
#include <tchar.h>
#include <share.h>

#define nullptr NULL
#define  FOLDERPATH_MAX			1024
#define  MAX_STRLEN				1024

static int TxtFileMark = 1;
void DumpErrorMes(LPCTSTR lpszFunction)
{
#ifdef DebugMesPrint

	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();
	CString MesBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, nullptr);

	if (dw == ERROR_SUCCESS)
	{
		MesBuf.Format(_T("%s succeed:  %d: %s"), lpszFunction, dw, lpMsgBuf);
	}
	else
	{
		MesBuf.Format(_T("%s failed with error %d: %s"), lpszFunction, dw, lpMsgBuf);
	}

	//AfxMessageBox(MesBuf);
	DebugMesBox(MesBuf);
	LocalFree(lpMsgBuf);
	//ExitProcess(dw);

#endif
}

void DebugLogOutput(LPCTSTR lpszMessage)
{
#ifdef DebugMesPrint
	LogOutput(lpszMessage);
#endif
}

void DebugMesBox(LPCTSTR lpszMessage)
{
#ifdef	 DebugMesPrint
	//BOOL SaveLog(LPSTR pInputData, DWORD dwFileSize, const char* pFileTitle);
	// 输出到日志文件
	LogOutput(lpszMessage);

	AfxMessageBox(lpszMessage);
	//WriteLogFile(lpszMessage);
#endif
}

// 将日志信息写入txt
void WriteLogFile(CString strLog, BOOL bDump) {
	//#ifdef WriteLog
	// 只有调试版本才执行下列操作
#ifdef _DEBUG
	// 在执行任何操作之前立即捕获错误码
	DWORD dwError = 0;
	if (bDump) {
		dwError = GetLastError(); // 获取上一个API调用的错误码
	}

	// 获取系统时间
	SYSTEMTIME t;
	GetLocalTime(&t);
	CString strtime;
	strtime.Format(_T("%02d:%02d    "), t.wHour, t.wMinute);
	CString TxtFile = _T("Log");
	CString strAppName;

	// 获取日志文件的目录
	GetModuleFileName(nullptr, strAppName.GetBuffer(FOLDERPATH_MAX), FOLDERPATH_MAX);
	strAppName.ReleaseBuffer();
	int nPos = strAppName.ReverseFind('\\');
	strAppName = strAppName.Left(nPos + 1);

	// 格式化错误信息
	if (bDump && dwError != 0) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			nullptr, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf, 0, nullptr);
		strLog.Format(_T("%s failed with error %d: %s"), strLog, dwError, lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
	strLog = strtime + strLog + _T("\r\n\r\n");

	// 写入日志文件
	CString strFileName;
	for (int iNum = 1;; ++iNum) {
		strFileName.Format(_T("%s\\%s%04d%02d%02d(%d).txt"), strAppName, TxtFile, t.wYear, t.wMonth, t.wDay, iNum);
		CFile file;
		if (file.Open(strFileName, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate)) {
			ULONGLONG ileng = file.GetLength();
			if (ileng < 1048576) { // 文件大小小于1MB
				file.SeekToEnd();
				USES_CONVERSION;
				LPCSTR pMultiByteStr = T2A(strLog);
				file.Write(pMultiByteStr, strlen(pMultiByteStr));
				file.Close();
				break;
			}
		}
	}
#endif // WriteLog
}

// 根据错误码获取错误信息
CString GetErrorMsg(DWORD dwErrorCode) {
	CString strMsg;
	LPVOID lpMsgBuf = NULL;

	// 尝试从系统错误消息中获取
	DWORD dwChars = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	if (dwChars == 0) {
		// 如果失败，尝试从 SetupAPI.dll 中获取
		HMODULE hModule = LoadLibraryEx(TEXT("Setupapi.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
		if (hModule != NULL) {
			dwChars = FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
				hModule, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
			FreeLibrary(hModule);
		}
	}

	if (dwChars != 0 && lpMsgBuf != NULL) {
		strMsg = (LPCTSTR)lpMsgBuf;
		LocalFree(lpMsgBuf);
	}
	else {
		// 如果仍然失败，生成默认错误消息
		strMsg.Format(_T("Unknown error code: %d (0x%08X)"), dwErrorCode, dwErrorCode);
	}

	return strMsg;
}
//CString GetErrorMsg(DWORD dwErrorCode) {
//	CString strMsg;
//	LPVOID lpMsgBuf;
//	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//		NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
//	strMsg = (LPCTSTR)lpMsgBuf;
//	LocalFree(lpMsgBuf);
//	return strMsg;
//}

void LogOutput(CString strMsg) {
	// 获取当前程序运作路径
	TCHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 为日志文件路径赋值
	CString strPath = szPath;
	int nPos = strPath.ReverseFind('\\');
	strPath = strPath.Left(nPos + 1);

	// 获取应用程序名称（不包含扩展名）
	CString appName = szPath + nPos + 1; // +1 跳过反斜线
	nPos = appName.ReverseFind('.');
	if (nPos != -1) {
		appName = appName.Left(nPos);  // 去掉扩展名
	}

	// 使用应用程序名称组织日志文件名称
	CString logFileName;
	logFileName.Format(_T("%s.log"), appName);
	strPath += logFileName;

	// 输出日志文件保存在当前程序路径下
	CString strFile = strPath;

	// 打开、创建文件
	FILE* fp = _tfsopen(strFile, _T("a, ccs=UTF-8"), _SH_DENYNO); // 使用UTF-8编码保存中文字符
	if (fp == NULL) {
		TRACE(_T("打开输出文件失败！\n"));
		return;
	}
	// 将日志信息写入文件中，格式化输出字符串
	SYSTEMTIME SysTime;
	GetLocalTime(&SysTime);
	CString strTime;
	strTime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d "),
		SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
	CString strLogMsg = strTime + strMsg + _T("\n");
	_fputts(strLogMsg, fp);

	// 关闭文件
	fclose(fp);
}

// 先获取错误码，再转换成错误信息，并将错误信息输出到日志文件
//void GetErrAndLog(CString strError) {
//	// DEBUG模式下才执行下列操作
//#ifdef _DEBUG
//
//	DWORD dwErrorCode = GetLastError();
//	CString strMsg = GetErrorMsg(dwErrorCode);
//
//	// 将错误码和错误解释拼接成一条完整的错误信息
//	CString strErrorCode;
//	strErrorCode.Format(_T("%d"), dwErrorCode);
//	CString strLogMsg = strError + _T("，错误码：") + strErrorCode + _T("，错误信息：") + strMsg;
//	LogOutput(strLogMsg);
//#endif
//}
void GetErrAndLog(CString strError) {
	// DEBUG模式下才执行下列操作
#ifdef _DEBUG

	DWORD dwErrorCode = GetLastError();
	CString strMsg = GetErrorMsg(dwErrorCode);

	// 将错误码和错误解释拼接成一条完整的错误信息
	CString strErrorCode;
	strErrorCode.Format(_T("%d (0x%08X)"), dwErrorCode, dwErrorCode);
	CString strLogMsg = strError + _T("，错误码：") + strErrorCode + _T("，错误信息：") + strMsg;
	LogOutput(strLogMsg);
#endif
}
