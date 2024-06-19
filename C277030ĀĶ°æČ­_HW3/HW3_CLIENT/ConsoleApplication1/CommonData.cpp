#include "CommonData.h"


VOID error_Quit(const wchar_t* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

VOID error_Display(const wchar_t* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	std::wcout << "[" << msg << "]" << (TCHAR*)lpMsgBuf;
	LocalFree(lpMsgBuf);
}