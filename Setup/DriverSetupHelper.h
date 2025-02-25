// DriverSetupHelper.h: interface for the CDriverSetupHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRIVERSETUPHELPER_H__00E8E58D_7C08_4280_B5BC_F999D436C018__INCLUDED_)
#define AFX_DRIVERSETUPHELPER_H__00E8E58D_7C08_4280_B5BC_F999D436C018__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "newdev.h"   // for the API UpdateDriverForPlugAndPlayDevices().
#include <setupapi.h> // for SetupDiXxx functions.

class CDriverSetupHelper
{
public:
	static BOOL FindExistingDevice(CString szHardwareId);
	//static BOOL Install(IN CString szInfFile, IN CString szHardwareID, IN BOOL bMultiInstance = TRUE OPTIONAL, OUT PBOOL pbRebootRequired = NULL OPTIONAL);
	static BOOL Install(IN CString szInfFile, IN CString szHardwareID);
	CDriverSetupHelper();
	virtual ~CDriverSetupHelper();
	//public:
	//	static BOOL InstallRootEnumeratedDriver(CString szInfFile,CString szHardwareID, OUT PBOOL pbRebootRequired=NULL);
};

#endif // !defined(AFX_DRIVERSETUPHELPER_H__00E8E58D_7C08_4280_B5BC_F999D436C018__INCLUDED_)
