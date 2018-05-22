// KeyMonitor.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <stdio.h>
#include <Windows.h>

#define SLEEP_TIME 2000 //���ʱ��
#define SERVICE_NAME TEXT("KeyMonitor")
bool brun = false;
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
	servicestatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP| SERVICE_ACCEPT_PAUSE_CONTINUE;//�ڱ�����ֻ����ϵͳ�ػ���ֹͣ�������ֿ�������
	servicestatus.dwWin32ExitCode = 0;
	servicestatus.dwServiceSpecificExitCode = 0;
	servicestatus.dwCheckPoint = 0;
	servicestatus.dwWaitHint = 0;
	hstatus = ::RegisterServiceCtrlHandler(SERVICE_NAME, CtrlHandler);

	if (hstatus == 0)
	{
		return;
	}

	//��SCM ��������״̬
	servicestatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hstatus, &servicestatus);
	//�ڴ˴��������Լ�ϣ���������Ĺ����������������Ĺ����ǻ�õ�ǰ���õ������������ڴ���Ϣ

}

void WINAPI CtrlHandler(DWORD request)
{

	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		brun = false;
		servicestatus.dwCurrentState = SERVICE_STOPPED;
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		brun = false;
		servicestatus.dwCurrentState = SERVICE_STOPPED;
		break;
	default:
		break;
	}
	SetServiceStatus(hstatus, &servicestatus);
}

BOOL IsInstalled()
{
	BOOL bResult = FALSE;

	//�򿪷�����ƹ�����  
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM != NULL)
	{
		//�򿪷���  
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

	//�򿪷�����ƹ�����  
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), SERVICE_NAME, MB_OK);
		return FALSE;
	}

	// Get the executable file path  
	TCHAR szFilePath[MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);

	//��������  
	SC_HANDLE hService = ::CreateService(
		hSCM, SERVICE_NAME, SERVICE_NAME,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, _T(""), NULL, NULL);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't create service"), SERVICE_NAME, MB_OK);
		return FALSE;
	}

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
		MessageBox(NULL, _T("Couldn't open service manager"), SERVICE_NAME, MB_OK);
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, SERVICE_NAME, SERVICE_STOP | DELETE);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't open service"), SERVICE_NAME, MB_OK);
		return FALSE;
	}
	SERVICE_STATUS status;
	::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	//ɾ������  
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
