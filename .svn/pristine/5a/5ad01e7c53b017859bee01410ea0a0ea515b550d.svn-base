/************************************************************************

FileName:	LPTCtrl.h

Copyright (c) 2009-2012

Writer:	黄磊

create Date:	202.07.10

Rewriter:

Rewrite Date:

Impact:	LPT端口控制功能模块预定义头文件及常量

Main Content:
			GetLPTDeviceID();			// 获取并口设备ID
			OpenLPT();						// 打开并口
			WriteLPTPort();					// 往并口写入数据
			CloseLPTPort();				// 关闭并口句柄

/************************************************************************/
#pragma  once

#define  STR_DEVICE_ID    _T("START")						// 实达系列机型设备ID

// 打开并口
HANDLE OpenLPTPort(CString PortNum);
//枚举设备管理器,返回打开的并口，如果有设备ID返回可能为1.4...
CString QueryLPTPort(CString PortNum);
//从LPT读取回来的ID解析出打印机并口号
CString ResolveLPTPort(CString PrintId);
BOOL   ScanForHardwareChanges(TCHAR* pDeviceID);
//获取并口设备ID
BOOL IsSTDeviceID(TCHAR* lpLPTName, CString& PrintModel);

//从USB读取回来的ID解析出打印机型号
LPTSTR ResolvePrintId(CString PrintId);
//从USB读取回来的ID解析出仿真方式
LPTSTR ResolvePrintCommand(CString PrintId, CString print);
//从USB读取回来的ID解析出产家名称
LPTSTR ResolvePrintProvider(CString Provider);
