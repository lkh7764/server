#include "ClientAgent.h"
#include <string>
#include <conio.h>



ClientAgent::ClientAgent()
{
	setID();

	pos = { 0.0f, 0.0f, 0.0f };

	if (WSAStartup(MAKEWORD(2, 2), &Wsadata) != 0)
		return;
}

ClientAgent::ClientAgent(string id)
{
	char* id_c = nullptr;
	strcpy(id_c, id.c_str());

	if (checkID(clientID, id_c) == false)
		setID();

	pos = { 0.0f, 0.0f, 0.0f };
}

ClientAgent::~ClientAgent()
{
	memset(clientID, 0, ID_SIZE);
	pos = { 0.0f, 0.0f, 0.0f };

	closesocket(connectSock);
	WSACleanup();
}




int ClientAgent::checkHeader(char* buffer)
{
#if TEST==0
	cout << "\n\n-------------CHECK HEADER START-------------\n" << endl;
#endif

	int h;
	memcpy(&h, buffer + sizeof(int), sizeof(int));

#if TEST==0
	cout << "[PacketHeader]::" << h << endl;
	cout << "\n-------------CHECK HEADER FINISH-------------\n\n" << endl;
#endif
	return h;
}

void ClientAgent::deserialize(Packet* p, char* buffer)
{
	cout << "deserialize" << endl;

	int len;
	memcpy(&len, buffer, sizeof(int));
	//cout << "length:" << len << endl;

	int h;
	memcpy(&h, buffer + sizeof(int), sizeof(int));
	//cout << "header:" << h << endl;

	char id[ID_SIZE];
	memcpy(id, buffer + sizeof(int) * 2, sizeof(char) * ID_SIZE);
	//cout << "id:" << id[0] << id[1] << id[2] << id[3] << id[4] << id[5] << id[6] << endl;
	//cout << "askiNum-id: "  << 0 + id[0] << endl;

	switch (h) {
	case ACK_CON:
		cout << "ACK_CON" << endl;

		bool con;
		memcpy(&con, buffer + sizeof(int) * 2 + sizeof(char) * ID_SIZE, sizeof(bool));
		//cout << "connection: " << con << endl;

		int ct[6];
		memcpy(ct, buffer + sizeof(int) * 2 + sizeof(char) * ID_SIZE + sizeof(bool), sizeof(int)*6);
		//cout << "connection Time-year: " << ct[0] << endl;
		//cout << "connection Time-second: " << ct[5] << endl;

		p = new ACK_CONPacket(id, con, ct);
		break;

	case ACK_MOV:
		cout << "ACK_MOVE" << endl;

		Vector3 position;
		memcpy(&position, buffer + sizeof(int) * 2 + sizeof(char) * ID_SIZE, sizeof(Vector3));
		cout << position.x << " | " << position.y << " | " << position.z << endl;

		char addr_ip[IPADDR_SIZE];
		memcpy(addr_ip, buffer + sizeof(int) * 2 + sizeof(char) * ID_SIZE + sizeof(Vector3), IPADDR_SIZE);
		//cout << "IP Addr: " << addr_ip << endl;

		p = new ACK_MovePacket(id, position, addr_ip);
		break;

	case CHATTING:
		cout << "CHATTING" << endl;

		char otherid[ID_SIZE];
		memcpy(otherid, buffer + sizeof(int) * 2 + sizeof(char) * ID_SIZE, sizeof(char) * ID_SIZE);
		cout << "otherid:" << otherid[0] << otherid[1] << otherid[2] << otherid[3] << otherid[4] << otherid[5] << otherid[6] << endl;
		cout << 0 + otherid[0] << endl;

		char msg_c[MSG_SIZE];
		memcpy(&msg_c, buffer + sizeof(int) * 2 + sizeof(char) * 2 * ID_SIZE, MSG_SIZE);
		cout << "message:" << msg_c << endl;

		p = new ChatPacket(id, otherid, msg_c);
		break;

	default:
		cout << "wrong data" << endl;
		h = P_ERROR;
		break;
	}
}




void ClientAgent::setID()
{
	clientID[0] = 'A' + rand() % 26;
	for (int i = 1; i < ID_SIZE; i++)
		clientID[i] = '0' + rand() % 10;
}

VOID ClientAgent::chatting()
{
#if TEST==0
	cout << "\n\n-------------CHATTING START-------------\n" << endl;
#endif

	string str;
	cout << "Enter client ID :" << endl;
	cin >> str;

	string str1;
	cout << "Enter message :" << endl;
	cin >> str1;

	char testid[ID_SIZE] = { 'K','0','0' ,'0' ,'0' ,'0' ,'0' };
	memcpy(testid, str.c_str(), ID_SIZE);

	ChatPacket* packet = new ChatPacket(clientID, testid, str1);
	size_t dataSize = 0;

	int len = packet->get_length();
	if (len == -1) {
		cout << "PACKET GENARATION FAILED\n\n" << endl;
		return;
	}

	packet->serialize(buffer, dataSize);

	send(connectSock, buffer, dataSize, 0);
#if TEST==0
	char out_id[ID_SIZE + 1];
	memcpy(out_id, clientID, ID_SIZE);
	out_id[ID_SIZE] = '\0';

	cout << "\n\n[" << out_id << "]: send CHATTING\n\n" << endl;
#endif

	packet->~ChatPacket();
	packet = nullptr;

#if TEST==0
	cout << "\n-------------CHATTING FINISH-------------\n\n" << endl;
#endif
}

VOID ClientAgent::move(Vector3 dir)
{
#if TEST==0
	cout << "\n\n-------------MOVE START-------------\n" << endl;
#endif

	pos.x += dir.x;	pos.y += dir.y;	pos.z += dir.z;

	MovePacket* packet = new MovePacket(clientID, pos);
	size_t dataSize = 0;

	int len = packet->get_length();
	if (len == -1) {
		cout << "PACKET GENARATION FAILED\n\n" << endl;
		return;
	}

	memset(buffer, 0, BUFFER_SIZE);
	packet->serialize(buffer, dataSize);

	send(connectSock, buffer, dataSize, 0);
#if TEST==0
	char out_id[ID_SIZE + 1];
	memcpy(out_id, clientID, ID_SIZE);
	out_id[ID_SIZE] = '\0';

	cout << "\n\n[" << out_id << "]: send MovePacket\n\n" << endl;
#endif

	packet->~MovePacket();
	packet = nullptr;

#if TEST==0
	cout << "\n-------------MOVE FINISH-------------\n\n" << endl;
#endif
}




void ClientAgent::sendPacket_REQ_CON()
{
#if TEST==0
	cout << "\n\n-------------SEND REQCON START-------------\n" << endl;
#endif

	CONPacket* packet = new CONPacket(clientID, true);
	size_t dataSize = 0;

	int len = packet->get_length();
	if (len == -1) {
		cout << "PACKET GENARATION FAILED\n\n" << endl;
		return;
	}

	packet->serialize(buffer, dataSize);

	send(connectSock, buffer, dataSize, 0);
#if TEST==0
	char out_id[ID_SIZE + 1];
	memcpy(out_id, clientID, ID_SIZE);
	out_id[ID_SIZE] = '\0';

	cout << "\n\n[" << out_id << "]: send REQ_CON\n\n" << endl;
#endif

	packet->~CONPacket();
	packet = nullptr;

#if TEST==0
	cout << "\n-------------SEND REQCON FINISH-------------\n\n" << endl;
#endif
}

void ClientAgent::print_CHAT(char* buff)
{
#if TEST==0
	cout << "\n\n-------------PRINT CHATTING START-------------\n" << endl;
#endif

	char otherid[ID_SIZE + 1];
	memcpy(otherid, buff + sizeof(int) * 2 + sizeof(char) * ID_SIZE, sizeof(char) * ID_SIZE);
	otherid[ID_SIZE] = '\0';

	char msg_c[MSG_SIZE];
	memcpy(&msg_c, buff + sizeof(int) * 2 + sizeof(char) * 2 * ID_SIZE, MSG_SIZE);

	cout << "\n\n----------------CHAT----------------\n\n["
		<< otherid << "] :::: " << msg_c
		<< "\n\n---------------------------------------\n\n" << endl;

#if TEST==0
	cout << "\n-------------PRINT CHATTING FINISH-------------\n\n" << endl;
#endif
}

void ClientAgent::print_ACKMOV(char* buff)
{
#if TEST==0
	cout << "\n\n-------------PRINT ACKMOV START-------------\n" << endl;
#endif

	char otherid[ID_SIZE + 1];
	memcpy(otherid, buff + sizeof(int) * 2, sizeof(char) * ID_SIZE);
	otherid[ID_SIZE] = '\0';

	Vector3 otherpos;
	memcpy(&otherpos, buff + sizeof(int) * 2 + sizeof(char) * ID_SIZE, sizeof(Vector3));

	char otherip[IPADDR_SIZE + 1];
	memcpy(otherip, buff + sizeof(int) * 2 + sizeof(char) * ID_SIZE + sizeof(Vector3), IPADDR_SIZE);
	otherip[IPADDR_SIZE] = '\0';

	cout		<< "\n\n----------------SOMEONE MOVE----------------\n\n"
				<< "[MOVED CLIENT] : " << otherid << "\n"
				<< "[CLIENT`S IP] : " << otherip << "\n"
				<< "[CLIENT`S POSITION] : (" << otherpos.x << ", " << otherpos.y << ", " << otherpos.z << ")\n"
				<< "\n\n-------------------------------------------------------\n\n" << endl;

#if TEST==0
	cout << "\n-------------PRINT ACKMOV FINISH-------------\n\n" << endl;
#endif
}

void ClientAgent::print_ACKCON(char* buff)
{
#if TEST==0
	cout << "\n\n-------------PRINT ACKCON START-------------\n" << endl;
#endif

	char otherid[ID_SIZE + 1];
	memcpy(otherid, buff + sizeof(int) * 2, sizeof(char) * ID_SIZE);
	otherid[ID_SIZE] = '\0';

	bool con;
	memcpy(&con, buff + COMDATA_SIZE - 1, 1);

	int times[6];
	memcpy(times, buff + COMDATA_SIZE, 4 * 6);

	string str;
	string str2;
	if (con == true) {
		str = " is connected to server.";
		str2 = "[CONNECTION TIME]: ";
	}
	else {
		str = " is disconnected to server.";
		str2 = "[DISCONNECTION TIME]: ";
	}

	cout << "\'" << otherid << "\'" << str << endl;
	cout << str2
		<< times[0] << ". " << times[1] << ". " << times[2] << ". " << times[3] << ":" << times[4] << ":" << times[5]
		<< endl;

#if TEST==0
	cout << "\n-------------PRINT ACKCON FINISH-------------\n\n" << endl;
#endif
}



VOID ClientAgent::setReadyState()
{
	// socket()
	connectSock = socket(AF_INET, SOCK_STREAM, 0);
	if (connectSock == INVALID_SOCKET)
		error_quit(_T("socket()"));
}

VOID ClientAgent::communicate()
{
	INT			retval;
	HANDLE hThread[2];

	// connect()
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	serverAddr.sin_port = htons(SERVER_PORT);
	retval = connect(connectSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR)
		error_quit(_T("connect()"));

	sendPacket_REQ_CON();

	hThread[0] = CreateThread(NULL, 0, SendThread,  this, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, RecvThread, this, 0, NULL);
	//WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	WaitForSingleObject(hThread[0], INFINITE);

	closesocket(connectSock);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
}




DWORD WINAPI ClientAgent::SendThread(LPVOID lpParam)
{
	ClientAgent* This = (ClientAgent*)lpParam;
	SOCKET sock = This->serverSock;
	INT addressLen;

	INT retval;
	SOCKADDR_IN threadSocketAddress;

	addressLen = sizeof(SOCKADDR_IN);
	getpeername(sock, (SOCKADDR*)&threadSocketAddress, &addressLen);

	char c;

	while (1)
	{
		c = _getch();
		if (c == 'q' || c == 'Q') break;
		if (c == 'c' || c == 'C') This->chatting();
		if (c == 'w' || c == 'W') This->move({ 0.0f, 0.0f, 1.0f });
		if (c == 'a' || c == 'A') This->move({ -1.0f, 0.0f, 0.0f });
		if (c == 's' || c == 'S') This->move({ 0.0f, 0.0f, -1.0f });
		if (c == 'd' || c == 'D') This->move({ 1.0f, 0.0f, 0.0f });
	}

	closesocket(sock);
	cout << "test: close cnt" << endl;

	return -1;
}

DWORD WINAPI ClientAgent::RecvThread(LPVOID lpParam)
{
	ClientAgent*		This = (ClientAgent*)lpParam;
	SOCKET				sock = This->connectSock;
	INT						addressLen;

	INT						retval;
	SOCKADDR_IN	threadSocketAddress;
	CHAR					Buffer[BUFFER_SIZE];
	Packet*				sample;

	addressLen = sizeof(SOCKADDR_IN);
	getpeername(sock, (SOCKADDR*)&threadSocketAddress, &addressLen);

	while (1) 
	{
		sample = nullptr;
		memset(Buffer, 0, BUFFER_SIZE);

		retval = recv(sock, Buffer, BUFFER_SIZE, 0);
		if (retval == SOCKET_ERROR) {
			error_display(_T("recv()"));
			continue;
		}
		else if (retval == 0) {
			cout << "\n\n\nserver sock closed\n\n\n" << endl;
			break;
		}

		int h = This->checkHeader(Buffer);
		if (h == P_ERROR) {
			cout << "HEADER: ERROR\n" << endl;
			continue;
		}

		//This->deserialize(sample, Buffer);
		switch (h) {
		case ACK_CON:
#if TEST==0
			cout << "\nHEADER: ACK_CON\n" << endl;
#endif
			This->print_ACKCON(Buffer);
			break;
		case ACK_MOV:
#if TEST==0
			cout << "\nHEADER: ACK_MOV\n" << endl;
#endif
			This->print_ACKMOV(Buffer);
			break;
		case CHATTING:
#if TEST==0
			cout << "\nHEADER: CHATTING\n" << endl;
#endif
			This->print_CHAT(Buffer);
			break;
		case ID_INFO:
#if TEST==0
			cout << "\nHEADER: ID_INFO\n" << endl;
#endif
			break;
		}
	}

	return -1;
}

