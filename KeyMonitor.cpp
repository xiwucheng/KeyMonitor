// KeyMonitor.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include "KeyServer.h"

#define SLEEP_TIME 2000 //间隔时间
#define SERVICE_NAME TEXT("KeyMonitor")
CKeyServer* pServer = NULL;
SERVICE_STATUS servicestatus;
SERVICE_STATUS_HANDLE hstatus;
void WINAPI ServiceMain();
void WINAPI CtrlHandler(DWORD request);
void LogEvent(LPCTSTR pFormat, ...)
{
	TCHAR    chMsg[256];
	HANDLE  hEventSource;
	LPTSTR  lpszStrings[1];
	va_list pArg;

	va_start(pArg, pFormat);
	_vstprintf_s(chMsg, pFormat, pArg);
	va_end(pArg);

	lpszStrings[0] = chMsg;

	hEventSource = RegisterEventSource(NULL, SERVICE_NAME);
	if (hEventSource != NULL)
	{
		ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, (LPCTSTR*)&lpszStrings[0], NULL);
		DeregisterEventSource(hEventSource);
	}
}

void WINAPI ServiceMain()
{
	servicestatus.dwServiceType = SERVICE_WIN32;
	servicestatus.dwCurrentState = SERVICE_START_PENDING;
	servicestatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP| SERVICE_ACCEPT_PAUSE_CONTINUE;//在本例中只接受系统关机和停止服务两种控制命令
	servicestatus.dwWin32ExitCode = 0;
	servicestatus.dwServiceSpecificExitCode = 0;
	servicestatus.dwCheckPoint = 0;
	servicestatus.dwWaitHint = 1000;
	hstatus = ::RegisterServiceCtrlHandler(SERVICE_NAME, CtrlHandler);

	pServer = new CKeyServer();
	pServer->Init();
	//-------------------------------------------------------------------//

	//向SCM 报告运行状态
	servicestatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hstatus, &servicestatus);
	//在此处添加你自己希望服务做的工作，在这里我做的工作是获得当前可用的物理和虚拟内存信息
	LogEvent(_T("KeyMonitor Service is running!"));

}

void WINAPI CtrlHandler(DWORD request)
{

	switch (request)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		pServer->Uninit();
		servicestatus.dwCurrentState = SERVICE_STOPPED;
		break;
	case SERVICE_CONTROL_CONTINUE:
		servicestatus.dwCurrentState = SERVICE_RUNNING;
		break;
	case SERVICE_CONTROL_PAUSE:
		servicestatus.dwCurrentState = SERVICE_PAUSED;
		break;
	default:
		break;
	}
	SetServiceStatus(hstatus, &servicestatus);
}

BOOL IsInstalled()
{
	BOOL bResult = FALSE;

	//打开服务控制管理器  
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM != NULL)
	{
		//打开服务  
		SC_HANDLE hService = ::OpenService(hSCM, SERVICE_NAME, SERVICE_QUERY_CONFIG);
		if (hService != NULL)
		{
			bResult = TRUE;
			::CloseServiceHandle(hService);
		}
		::CloseServiceHandle(hSCM);
	}
	return bResult;
}

BOOL Install()
{
	if (IsInstalled())
		return TRUE;

	//打开服务控制管理器  
	SERVICE_DESCRIPTION sd = { TEXT("OEM Activation 3.0 Key Collection Service")};
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		LogEvent(_T("Couldn't open service manager"));
		return FALSE;
	}

	// Get the executable file path  
	TCHAR szFilePath[MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);

	//创建服务  
	SC_HANDLE hService = ::CreateService(
		hSCM, SERVICE_NAME, SERVICE_NAME,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, _T(""), NULL, NULL);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		LogEvent(_T("Couldn't create service"));
		return FALSE;
	}
	ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &sd);
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);
	return TRUE;
}

BOOL Uninstall()
{
	if (!IsInstalled())
		return TRUE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM == NULL)
	{
		LogEvent(_T("Couldn't open service manager"));
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, SERVICE_NAME, SERVICE_STOP | DELETE);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		LogEvent(_T("Couldn't open service"));
		return FALSE;
	}
	SERVICE_STATUS status;
	::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	//删除服务  
	BOOL bDelete = ::DeleteService(hService);
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);

	if (bDelete)
		return TRUE;

	LogEvent(_T("Service could not be deleted"));
	return FALSE;
}

int _tmain(int argc, TCHAR** argv)
{
	//---------------------------------------------
#if 0
	pServer = new CKeyServer();
	pServer->Init();
	while (1) Sleep(500);
	//pServer->ExecuteSQL(TEXT("INSERT CollectedKeyInfo VALUES('11111','22222','33333','44444','55555','66666','77777',GETDATE())"));
	//......
	pServer->Uninit();
#endif
	//---------------------------------------------
	SERVICE_TABLE_ENTRY entrytable[2];

	entrytable[0].lpServiceName = SERVICE_NAME;

	entrytable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

	entrytable[1].lpServiceName = NULL;

	entrytable[1].lpServiceProc = NULL;

	if (argc == 2)
	{
		if (_tcsicmp(argv[1], TEXT("/install")) == 0)
		{
			Install();
		}
		else if (_tcsicmp(argv[1], TEXT("/uninstall")) == 0)
		{
			Uninstall();
		}
	}
	else
	{
		if (!::StartServiceCtrlDispatcher(entrytable))
		{
			LogEvent(_T("Register Service Main Function Error!"));
		}
	}
	return 0;
}

