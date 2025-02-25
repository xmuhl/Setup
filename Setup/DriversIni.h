#pragma once

#include "UDefault.h"
class CDriversIni
{
public:
	CDriversIni();
	~CDriversIni();

public:

	CString  m_strFilePath;   //存放ini文件路径

	// 查询ini表文件是否存在，存在返回TRUE,否则FALSE
	BOOL GetFile();
	int GetAllSeries(Serives* name);  //获取所有系列名字
	int GetAllPrintModel(CString  seriver, PrinterDefInfo* name);  //获取该系列的所有打印机名字
	void GetGPDandPrintModel(TCHAR* lpReturnstring, PrinterDefInfo* Dirver);  //将键值分离为驱动名称和对应的GPD

	bool GetPrintInstallFile(PrinterDefInfo* Dirver);

	void GetSetting();

	//OtherCmdInstallInfo * GetCmdInstallInfo(CString Info);
};
