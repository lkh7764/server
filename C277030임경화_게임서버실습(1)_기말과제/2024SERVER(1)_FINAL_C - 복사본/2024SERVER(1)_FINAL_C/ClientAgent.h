#pragma once
#include "Packet.h"


class ClientAgent
{
private:
	// attribute
	WSADATA			Wsadata;
	SOCKET				connectSock;

	SOCKET				serverSock;
	SOCKADDR_IN	serverAddr;
	
	char						clientID[ID_SIZE];
	Vector3				pos;
	char						buffer[BUFFER_SIZE];

	// method
	int						checkHeader(char* buffer);
	void						deserialize(Packet* p, char* buffer);

	void						setID();
	VOID					chatting();
	VOID					move(Vector3 dir);
	void						sendPacket_REQ_CON();

	void						print_CHAT(char* buff);
	void						print_ACKMOV(char* buff);
	void						print_ACKCON(char* buff);

public:
	ClientAgent();
	ClientAgent(string id);
	~ClientAgent();

	VOID 			setReadyState();
	VOID			communicate();

	static DWORD WINAPI	SendThread(LPVOID lpParam);
	static DWORD WINAPI	RecvThread(LPVOID lpParam);
};



