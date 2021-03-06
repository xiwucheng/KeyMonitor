#include "stdafx.h"
#include "KeyServer.h"

CKeyServer::CKeyServer()
{
	CoInitializeEx(NULL,COINIT_MULTITHREADED);
	m_ListKeyInfo.clear();
	try
	{
		m_pConn.CreateInstance(__uuidof(Connection));
	}
	catch (_com_error e)
	{
		OutputDebugString(e.ErrorMessage());
	}

	try
	{
		if (m_pConn->GetState() & adStateOpen)
		{
			m_pConn->Close();
		}
		m_pConn->Open(TEXT("Provider=SQLOLEDB.1;Data Source=localhost;Initial Catalog=KeyCollection;User ID=sa;Password=!Flex123"), TEXT(""), TEXT(""), adModeUnknown);
	}
	catch (_com_error e)
	{
		OutputDebugString(e.ErrorMessage());
	}
}

CKeyServer::~CKeyServer()
{
	try
	{
		if (m_pConn->GetState() & adStateOpen)
		{
			m_pConn->Close();
		}
		m_pConn.Release();
	}
	catch (_com_error e)
	{
		OutputDebugString(e.ErrorMessage());
	}
	CoUninitialize();
}

void CKeyServer::ExecuteSQL(LPCTSTR lpszCmdText)
{
	try
	{
		if (m_pConn->GetState() & adStateOpen)
		{
			m_pConn->Execute(lpszCmdText, 0, adCmdText);
		}
	}
	catch (_com_error e)
	{
		OutputDebugString(e.ErrorMessage());
	}
}

void CALLBACK CKeyServer::OnClientConnect(void* pOwner, CTCPCustom* pCustom)
{
	CKeyServer* p = (CKeyServer*)pOwner;
	CKeyInfo* pKeyInfo = new CKeyInfo();
	pKeyInfo->socket = pCustom->GetSocket();
	p->m_ListKeyInfo.push_back(pKeyInfo);
}

void CALLBACK CKeyServer::OnClientClose(void* pOwner, CTCPCustom* pCustom)
{
	CKeyServer* p = (CKeyServer*)pOwner;
	list<CKeyInfo*>::iterator it;
	for (it = p->m_ListKeyInfo.begin(); it != p->m_ListKeyInfo.end(); it++)
	{
		CKeyInfo* pKeyInfo = *it;
		if (pKeyInfo && pKeyInfo->socket == pCustom->GetSocket())
		{
			p->m_ListKeyInfo.remove(*it);
			delete pKeyInfo;
			break;
		}
	}

}

int CALLBACK CKeyServer::OnClientRead(void* pOwner, CTCPCustom* pCustom, char* buf, int len)
{
	CKeyServer* p = (CKeyServer*)pOwner;
	char buff[1024] = { 0 };
	TCHAR strSQL[512] = { 0 };
	int rdx = 0;

	list<CKeyInfo*>::iterator it;
	for (it = p->m_ListKeyInfo.begin(); it != p->m_ListKeyInfo.end(); it++)
	{
		CKeyInfo* pKeyInfo = *it;
		if (pKeyInfo && pKeyInfo->socket == pCustom->GetSocket())
		{
			if (len == 9 && strcmp(buf, "handshake") == 0)
			{
				pCustom->SendData("granted", strlen("granted"));
				break;
			}
			else if (len == sizeof(KeyInfo))
			{
				memcpy(&pKeyInfo->m_KeyInfo, buf, len);
				pKeyInfo->init_crc_table();
				DWORD crc = pKeyInfo->CRC32(0xFFFFFFFF, (BYTE*)&pKeyInfo->m_KeyInfo, sizeof(KeyInfo) - 4);
				if (crc == pKeyInfo->m_KeyInfo.CRC)
				{
					_stprintf_s(strSQL, TEXT("INSERT INTO CollectedKeyInfo VALUES('%S','%S','%S','%S','%S','%S','%S',GETDATE())"),
						pKeyInfo->m_KeyInfo.PKID, pKeyInfo->m_KeyInfo.KEY, pKeyInfo->m_KeyInfo.SSN,
						pKeyInfo->m_KeyInfo.BSN, pKeyInfo->m_KeyInfo.WIFI, pKeyInfo->m_KeyInfo.BT,
						pKeyInfo->m_KeyInfo.IMEI);
					//pCustom->SendData("techvision", strlen("techvision"));
					p->ExecuteSQL(strSQL);
					OutputDebugString(strSQL);
				}
				break;
			}
			else
			{
				return 1;
			}
		}
	}
	return 0;
}

void CALLBACK CKeyServer::OnClientError(void* pOwner, CTCPCustom* pCustom, int nErrorCode)
{
	CKeyServer* p = (CKeyServer*)pOwner;
	list<CKeyInfo*>::iterator it;
	for (it = p->m_ListKeyInfo.begin(); it != p->m_ListKeyInfo.end(); it++)
	{
		CKeyInfo* pKeyInfo = *it;
		if (pKeyInfo && pKeyInfo->socket == pCustom->GetSocket())
		{
			p->m_ListKeyInfo.remove(*it);
			delete pKeyInfo;
			break;
		}
	}
}

void CALLBACK CKeyServer::OnServerError(void* pOwner, CTCPServer* pServer, int nErrorCode)
{
	CKeyServer* p = (CKeyServer*)pOwner;
}

BOOL CKeyServer::Init()
{
	m_pServer = new CTCPServer(this);
	m_pServer->m_OnClientConnect = OnClientConnect;
	m_pServer->m_OnClientClose = OnClientClose;
	m_pServer->m_OnClientRead = OnClientRead;
	m_pServer->m_OnClientError = OnClientError;
	m_pServer->m_OnServerError = OnServerError;
	m_bStart = (m_pServer->Open(4000) == 1) ? TRUE : FALSE;
	return m_bStart;
}

void CKeyServer::Uninit()
{
	if (m_bStart)
	{
		m_pServer->Close();
		m_bStart = FALSE;
	}
	delete m_pServer;
}