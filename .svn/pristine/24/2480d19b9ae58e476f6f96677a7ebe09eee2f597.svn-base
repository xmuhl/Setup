#include "stdafx.h"
#include "PAVServiceOper.h"
#include <winsvc.h>

CPAVServiceOper::CPAVServiceOper()
{
}

CPAVServiceOper::~CPAVServiceOper()
{
}

// 开启服务，成功返回1，失败返回0
bool CPAVServiceOper::RunService()
{
	SC_HANDLE hSC = ::OpenSCManager(NULL, NULL, GENERIC_EXECUTE);

	if (hSC == NULL)
	{
		return 0;
	}

	SC_HANDLE hSvc = ::OpenService(hSC, L"PcaSvc", SERVICE_START | SERVICE_QUERY_STATUS | SERVICE_STOP);

	if (hSvc == NULL)
	{
		::CloseServiceHandle(hSC);
		return 0;
	}

	SERVICE_STATUS status;
	if (::QueryServiceStatus(hSvc, &status) == FALSE)
	{
		::CloseServiceHandle(hSvc);
		::CloseServiceHandle(hSC);
		return 0;
	}

	if (status.dwCurrentState == SERVICE_STOPPED)
	{
		// 启动服务
		if (::StartService(hSvc, NULL, NULL) == FALSE)
		{
			TRACE("start service error。");
			::CloseServiceHandle(hSvc);
			::CloseServiceHandle(hSC);
			return 0;
		}
		// 等待服务启动
		while (::QueryServiceStatus(hSvc, &status) == TRUE)
		{
			::Sleep(status.dwWaitHint);
			if (status.dwCurrentState == SERVICE_RUNNING)
			{
				OutputDebugString(L"start success。");
				::CloseServiceHandle(hSvc);
				::CloseServiceHandle(hSC);
				return 0;
			}
		}
	}

	::CloseServiceHandle(hSvc);
	::CloseServiceHandle(hSC);

	return 1;
}
//关闭服务，成功返回1，失败返回0
bool  CPAVServiceOper::CloseService()
{
	SC_HANDLE hSC = ::OpenSCManager(NULL, NULL, GENERIC_EXECUTE);

	if (hSC == NULL)
	{
		return 0;
	}

	SC_HANDLE hSvc = ::OpenService(hSC, L"PcaSvc", SERVICE_START | SERVICE_QUERY_STATUS | SERVICE_STOP);

	if (hSvc == NULL)
	{
		::CloseServiceHandle(hSC);
		return 0;
	}

	SERVICE_STATUS status;
	if (::QueryServiceStatus(hSvc, &status) == FALSE)
	{
		::CloseServiceHandle(hSvc);
		::CloseServiceHandle(hSC);
		return 0;
	}

	//如果处于停止状态则启动服务，否则停止服务。
	if (status.dwCurrentState == SERVICE_RUNNING)
	{
		// 停止服务
		if (::ControlService(hSvc,
			SERVICE_CONTROL_STOP, &status) == FALSE)
		{
			TRACE("stop service error。");
			::CloseServiceHandle(hSvc);
			::CloseServiceHandle(hSC);
			return 0;
		}
		// 等待服务停止
		while (::QueryServiceStatus(hSvc, &status) == TRUE)
		{
			::Sleep(status.dwWaitHint);
			if (status.dwCurrentState == SERVICE_STOPPED)
			{
				OutputDebugString(L"stop success。");
				::CloseServiceHandle(hSvc);
				::CloseServiceHandle(hSC);
				return 0;
			}
		}
	}
	return 1;
}

//关闭服务
UINT ClosePAV(LPVOID pParam)
{
	//关闭PAV服务
	CPAVServiceOper  serviseoper;
	serviseoper.CloseService();
	return 0;
}