// TCPSocket.h: interface for the CTCPServer/CTCPCustom/CTCPClient class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __TCPSOCKET_H__
#define __TCPSOCKET_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <winsock2.h>
#include <WS2tcpip.h>
#include <list>
using namespace std;
//#include <afxtempl.h>

class CTCPCustom;
class CTCPServer;

typedef void(CALLBACK* ONCLIENTCONNECT)(void* pOwner, CTCPCustom*);
typedef void(CALLBACK* ONCLIENTCLOSE)(void* pOwner, CTCPCustom*);
typedef int(CALLBACK* ONCLIENTREAD)(void* pOwner, CTCPCustom*, char* buf, int len);
typedef void(CALLBACK* ONCLIENTERROR)(void* pOwner, CTCPCustom*, int nErrorCode);
typedef void(CALLBACK* ONSERVERERROR)(void* pOwner, CTCPServer*, int nErrorCode);

class CTCPServer  
{
	friend class CTCPCustom;
public:
	CTCPServer(void* pOwner);
	virtual ~CTCPServer();
public:
	void RemoveClient(CTCPCustom* pClient);
	int Open(int nLocalPort);
	int Close();
	BOOL SendData(CTCPCustom* pCustom, const char* buf,int len);

	ONCLIENTCONNECT m_OnClientConnect;
	ONCLIENTCLOSE m_OnClientClose;
	ONCLIENTREAD m_OnClientRead;
	ONCLIENTERROR m_OnClientError;
	ONSERVERERROR m_OnServerError;

private:
	static DWORD SocketThreadFunc(LPVOID lParam);
	static DWORD ClientThreadFunc(LPVOID lParam);
	list <CTCPCustom*> m_ListClientSocket;
	SOCKET m_serverSocket;
	HANDLE m_serverThreadHandle,m_clientThreadHandle;
	HANDLE m_exitTcpThreadEvent;
	void* m_pOwner;
};

class CTCPCustom
{
	friend class CTCPServer;
public:
	CTCPCustom();
	virtual ~CTCPCustom();
public:
	BOOL Associate(CTCPServer* pTCPServer);
	BOOL Close();
	BOOL SendData(const char* buf,int len);
	char* GetHostIp();
	SOCKET GetSocket();

private:
	static DWORD SocketThreadFunc(LPVOID lParam);
	HANDLE m_exitCustomThreadEvent;
	HANDLE m_tcpThreadHandle;
	CTCPServer* m_pTCPServer;
	char m_remoteHost[128];
	DWORD m_remotePort;
	SOCKET m_socket;
};

typedef void(CALLBACK* ONDISCONNECT)(void* pOwner);
typedef void(CALLBACK* ONREAD)(void* pOwner, char* buf, int len);
typedef void(CALLBACK* ONERROR)(void* pOwner, int nErrorCode);

class CTCPClient  
{
public:
	CTCPClient(void* pOwner);
	virtual ~CTCPClient();
public:
	BOOL Open();
	BOOL Connect(char* m_remoteHost,int nPort);//remote host port
	BOOL SendData(const char* buf,int len);
	BOOL Close();
	BOOL IsOpen();
	//disconnect event
	ONDISCONNECT m_OnDisconnect;
	//receive event
	ONREAD m_OnRead;
	//error event
	ONERROR m_OnError;
private:
	static DWORD SocketThreadFunc(LPVOID lParam);

	SOCKET m_socket;
	HANDLE m_exitThreadEvent;
	HANDLE m_tcpThreadHandle;
	void*  m_pOwner;
	char m_recvBuf[4096];
	BOOL m_bIsOpen;
};

#endif //__TCPSOCKET_H__
