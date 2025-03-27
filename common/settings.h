#pragma once
#include <stdlib.h>
#include <string>

#define MAGIC "Hello, world!"

typedef struct Settings
{
	char MagicWord[32];
	char RemoteAddr[_MAX_PATH];
	int RemotePort;
	char Reserved[1024];
}Settings;

inline int MemoryFind(const char* szBuffer, const char* Key, int iBufferSize, int iKeySize)
{
	for (int i = 0; i < iBufferSize - iKeySize; ++i)
	{
		if (0 == memcmp(szBuffer + i, Key, iKeySize))
		{
			return i;
		}
	}
	return -1;
}

inline std::string UnicodeToASCII(const std::wstring& unicode_str) {
	std::string ascii_str;
	for (wchar_t wc : unicode_str) {
		if (wc <= 127) {  // �ж��Ƿ�����Ч�� ASCII �ַ�
			ascii_str.push_back(static_cast<char>(wc));
		}
		else {
			ascii_str.push_back('?');  // ��������� ASCII ��Χ������ '?' ���
		}
	}
	return ascii_str;
}
