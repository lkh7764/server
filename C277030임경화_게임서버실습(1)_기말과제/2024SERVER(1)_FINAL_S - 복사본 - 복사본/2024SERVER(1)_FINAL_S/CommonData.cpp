#include "CommonData.h"


void error_quit(const wchar_t* msg) 
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

void error_display(const wchar_t* msg)
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
	cout << "\n\n-------------ERROR DISPLAY START-------------\n" << endl;
	std::wcout << "[" << msg << "]\n" << (TCHAR*)lpMsgBuf;
	cout << "\n-------------ERROR DISPLAY FINISH-------------\n\n" << endl;
	LocalFree(lpMsgBuf);
}

bool checkID(char* buf, char id[ID_SIZE])
{
#if TEST==0
	cout << "\n\n-------------CHECK ID START-------------\n" << endl;
#endif

	for (int i = 0; i < ID_SIZE; i++) {
		switch (i) {
		case 0:
			if (id[i] < 'A' || id[i]>'Z') {
#if TEST==0
				cout << "[" << i  << "]" << "failed ::: "  << id[i] << endl;
				cout << "\n-------------CHECK ID FINISH-------------\n\n" << endl;
#endif
				return false;
			}

			*buf = id[i];
			break;
		default:
			if (id[i] < '0' || id[i]>'9') {
#if TEST==0
				cout << "[" << i << "]" << "failed" << endl;
				cout << "\n-------------CHECK ID FINISH-------------\n\n" << endl;
#endif
				return false;
			}

			*(buf + i) = id[i];
			break;
		}
	}
	
#if TEST==0
	cout << "\n-------------CHECK ID FINISH-------------\n\n" << endl;
#endif
	return true;
}