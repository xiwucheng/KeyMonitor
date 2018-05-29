#include "stdafx.h"
#include "KeyInfo.h"

CKeyInfo::CKeyInfo()
{
	socket = 0; 
	memset(&m_KeyInfo, 0, sizeof(m_KeyInfo));
}

CKeyInfo::~CKeyInfo()
{ 
	socket = 0;
	memset(&m_KeyInfo, 0, sizeof(m_KeyInfo));
}

void CKeyInfo::init_crc_table()
{
	DWORD c;
	DWORD i, j;

	for (i = 0; i < 256; i++) {
		c = (DWORD)i;
		for (j = 0; j < 8; j++) {
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		CRC_Table[i] = c;
	}
}

DWORD CKeyInfo::CRC32(DWORD crc, BYTE *buffer, DWORD size)
{
	DWORD i;
	for (i = 0; i < size; i++) {
		crc = CRC_Table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
	}
	return crc;
}

