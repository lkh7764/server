#pragma once
#include "CommonData.h"


class ClientAgent
{
private:
	WSADATA			Wsadata;
	SOCKET				connectSock;

	SOCKET				serverSock;
	SOCKADDR_IN	serverAddr;
	CHAR					buffer[BUFFERSIZE];

	INT						primeNum;

public:
	ClientAgent(int num) {
		primeNum = num;
		wcout << primeNum << endl;
		if (WSAStartup(MAKEWORD(2, 2), &Wsadata) != 0)
			return;
	}
	~ClientAgent()
	{
		closesocket(connectSock);
		WSACleanup();
	}

	VOID 			setReadyState();
	VOID			communicate();
	//static DWORD WINAPI	ClientThread(LPVOID lpParam);
};



//DWORD WINAPI ClientAgent::ClientThread(LPVOID lpParam) {}

