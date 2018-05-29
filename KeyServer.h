#pragma once
#include "TCPSocket.h"
#include "KeyInfo.h"
#include <list>
#import "c:\program files\common files\system\ado\msado15.dll" no_namespace rename("EOF","adoEOF")
using namespace std;

class CKeyServer
{
public:
	CKeyServer();
	virtual ~CKeyServer();
public:
	list <CKeyInfo*> m_ListKeyInfo;
	CTCPServer* m_pServer;
	BOOL m_bStart;
	BOOL Init();
	void Uninit();
	void ExecuteSQL(LPCTSTR lpszCmdText);
	static void CALLBACK OnClientConnect(void* pOwner, CTCPCustom* pCustom);
	static void CALLBACK OnClientClose(void* pOwner, CTCPCustom* pCustom);
	static int CALLBACK OnClientRead(void* pOwner, CTCPCustom* pCustom, char* buf, int len);
	static void CALLBACK OnClientError(void* pOwner, CTCPCustom* pCustom, int nErrorCode);
	static void CALLBACK OnServerError(void* pOwner, CTCPServer* pServer, int nErrorCode);
public:
	_ConnectionPtr m_pConn;
};