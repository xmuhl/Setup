#pragma once
#include "Setupapi.h"
class OnlineRun
{
public:
	OnlineRun();
	~OnlineRun();

	HDEVINFO m_hDevInfo;
	CString m_szInfFile;
	CString m_szCurHwID;
	SP_CLASSIMAGELIST_DATA m_sid;
	int m_nDeviceIndex;
	// 获取inf路径
	CString GetInfPath(LPCTSTR PointMark, BOOL OsX64, LPCTSTR infName);
	// 打开inf文件
	BOOL OpenInf(CString szInfFile);
	void Reset();//释放内存
	BOOL AddNewDevice(); //增加设备
};
