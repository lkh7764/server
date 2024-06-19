#pragma once
#include "Packet.h"


struct ClientInfo {
	SOCKET				sock;
	SOCKADDR_IN	addr;
	int						addrLen;
	char						id[ID_SIZE];
};


class ServerAgent
{
private:
	// Data for socket
	USHORT 			SERVERPORT;
	WSADATA			Wsadata;
	SOCKET				listenSock;
	SOCKADDR_IN	serverAddr;

	// Data for communication
	ClientInfo	 		clients[CLIENT_MAX];
	CHAR					buffer[BUFFER_SIZE];
	INT						CONCntsNum;

	HANDLE				hThreads[CLIENT_MAX];
	HANDLE				hMutex;

	int						checkHeader(char* buffer);
	void						deserialize(Packet* p, char* buffer);

	void						send_ACKCON(int i, char* buff);
	void						send_ACKMOV(int i, char* buff);
	void						send_CHAT(int i, char* buff);
	void						send_ACKCON_discon(int i, char* buff);

public:
	ServerAgent() {
		SERVERPORT = SERVER_PORT;
		if (WSAStartup(MAKEWORD(2, 2), &Wsadata) != 0)
			return;

		if (hMutex == NULL)
			return;

		CONCntsNum = 0;

		for (int i = 0; i < CLIENT_MAX; i++) {
			initializeCntInfo(i);
			hThreads[i] = NULL;
		}
	}
	~ServerAgent()
	{
		closesocket(listenSock);
		WSACleanup();
	}

	VOID	setReadyState();
	VOID	communicate();
	VOID	initializeCntInfo(int i);

	static DWORD WINAPI	SocketThread(LPVOID lpParam);
	static VOID							UpdateCntID(int i, char id[ID_SIZE]);
};

