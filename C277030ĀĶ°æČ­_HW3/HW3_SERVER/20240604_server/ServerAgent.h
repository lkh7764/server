#pragma once
#include "commonData.h"


class ServerAgent
{
private:
	// Data for socket
	USHORT 			SERVERPORT;
	WSADATA			Wsadata;
	SOCKET				listenSock;
	SOCKADDR_IN	serverAddr;

	// Data for communication
	SOCKET				clientSock;
	SOCKADDR_IN	clientAddr;
	INT						addrLen;
	CHAR					buffer[BUFFERSIZE];

	HANDLE				hEvent;

public:
	ServerAgent() {
		SERVERPORT = SERVER_PORT;
		if (WSAStartup(MAKEWORD(2, 2), &Wsadata) != 0)
			return;

		hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (hEvent == NULL)
			return;
	}
	~ServerAgent()
	{
		closesocket(listenSock);
		WSACleanup();
	}

	VOID	setReadyState();
	VOID	communicate();
	static DWORD WINAPI	SocketThread(LPVOID lpParam);
};

