#pragma once
#include <windows.h>
typedef struct _tagKeyInfo
{
	char BSN[32];
	char SSN[32];
	char PKID[32];
	char KEY[32];
	char WIFI[32];
	char BT[32];
	char IMEI[32];
	DWORD CRC;
}KeyInfo, *PKeyInfo;

class CKeyInfo
{
public:
	CKeyInfo();
	virtual ~CKeyInfo();
	SOCKET socket;
	KeyInfo m_KeyInfo;
public:
	DWORD CRC_Table[256];
	void init_crc_table();
	DWORD CRC32(DWORD crc, BYTE *buffer, DWORD size);

};
