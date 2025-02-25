// StrTranslate.cpp: 用于在宽字符和多字节字符之间转换的实现

#include "stdafx.h"
#include "StrTranslate.h"

// 从多字节转化为宽字节
LPWSTR MultiToWChar(LPSTR pMultiByteStr, int iMultiByte)
{
	LPWSTR pWideChartStr = NULL;
	int iWideChar = 0;

	// 第一次调用MultiByteToWideChar()，计算转化成Unico字符串所需缓存。注：多字节字符串长度需+1，字符串末尾的'\0'结束符要被计算在内。
	int iResult = MultiByteToWideChar(0, 0, pMultiByteStr, iMultiByte + 1, pWideChartStr, iWideChar);

	// 第二次调用MultiByteToWideChar()，利用第一次调用返回的缓存大小划分一块缓存区
	// 用于存放转换后的Unicode字符串

	pWideChartStr = new WCHAR[iResult];
	ZeroMemory(pWideChartStr, iResult * 2);
	MultiByteToWideChar(0, 0, pMultiByteStr, iMultiByte, pWideChartStr, iResult);

	return pWideChartStr;
}

//从宽字节转化为多字节
LPSTR WCharToMulti(CString pWCharStr)
{
	wchar_t* wstr = pWCharStr.GetBuffer(0);
	pWCharStr.ReleaseBuffer();

	int iWChar = pWCharStr.GetLength();

	LPSTR pMultiByteStr = NULL;
	int iMultiByte = 0;

	// 第一次调用WideCharToMultiByte()，计算转化成Unico字符串所需缓存
	int iResult = WideCharToMultiByte(0, 0, wstr, iWChar, pMultiByteStr, \
		iMultiByte, NULL, NULL);

	// 第二次调用WideCharToMultiByte()，利用第一次调用返回的缓存大小划分一块缓存区
	// 用于存放转换后的Anicode字符串
	iResult++;		// 字符串数量加一用于存放'\0'
	pMultiByteStr = new char[iResult];
	ZeroMemory(pMultiByteStr, iResult);
	//memset(pMultiByteStr, '\0', iResult);
	WideCharToMultiByte(0, 0, pWCharStr, iWChar, pMultiByteStr, iResult, \
		NULL, NULL);

	return pMultiByteStr;
}