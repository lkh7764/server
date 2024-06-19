#include "ServerAgent.h"



struct CONParam {
	ServerAgent*	sa;
	int						cntNum;
};

// 통신 준비: 소켓 생성 > 바인드 > 리슨
VOID ServerAgent::setReadyState()
{
	INT retval;

	// socket()
	listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSock == INVALID_SOCKET)
		error_quit(_T("socket()"));

	// bind()
	ZeroMemory(&serverAddr, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVERPORT);
	retval = bind(listenSock,
		(SOCKADDR*)&serverAddr,
		sizeof(serverAddr)
	);
	if (retval == SOCKET_ERROR)
		error_quit(_T("bind()"));

	// listen()
	retval = listen(listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		error_quit(_T("listen()"));
}

// 통신: 어셉트 > SocketThread() 실행
VOID ServerAgent::communicate()
{
	hMutex = CreateMutex(NULL, FALSE, NULL);

	while (1)
	{
		if (CONCntsNum >= CLIENT_MAX) continue;

		int num = 0;
		for (num; num < CLIENT_MAX; num++) 
			if (clients[num].sock == NULL) break;

		//Accept()
		clients[num].addrLen = sizeof(clients[num].addr);
		clients[num].sock = accept(listenSock, (SOCKADDR*)&clients[num].addr, &clients[num].addrLen);
		if (clients[num].sock == INVALID_SOCKET)
		{
			error_display(_T("Accept"));
			break;
		}

		//Displaying Client Display
		std::wcout << std::endl << _T("[TCP Server] Client Connected : IP Address = ")
			<< inet_ntoa(clients[num].addr.sin_addr)
			<< _T(", Port = ") << ntohs(clients[num].addr.sin_port) << std::endl;

		CONCntsNum++;

		CONParam p = { this, num };
		CONParam* pp = &p;

		//Create Thread
		hThreads[num] = CreateThread(NULL, 0, SocketThread, pp, 0, NULL);

		//SetEvent(hEvent);
		//WaitForSingleObject(hThreads[num], INFINITE);
	}
}


VOID ServerAgent::initializeCntInfo(int i) 
{
	clients[i].sock = NULL;
	memset(&clients[i].addr, 0, sizeof(clients[i].addr));
	clients[i].addrLen = -1;
	memset(&clients[i].id, 0, sizeof(clients[i].id));
}


void ServerAgent::deserialize(Packet* p, char* buffer)
{
	cout << "deserialize" << endl;

	int len;
	memcpy(&len, buffer, sizeof(int));
	//cout << "length:" << len << endl;

	int h;
	memcpy(&h, buffer + sizeof(int), sizeof(int));
	//cout << "header:" << h << endl;

	char id[ID_SIZE];
	memcpy(id, buffer + sizeof(int)*2, sizeof(char)*ID_SIZE);
	//cout << "id:" << id[0] << id[1] << id[2] << id[3] << id[4] << id[5] << id[6] << endl;
	//cout << "askiNum-id: "  << 0 + id[0] << endl;

	switch (h) {
	case REQ_CON:
		cout << "REQ_CON" << endl;

		bool con;
		memcpy(&con, buffer + sizeof(int) * 2 + sizeof(char) * ID_SIZE, sizeof(bool));
		cout << "connection: " << con << endl;

		p = new CONPacket(id, con);
		break;

	case REQ_MOV:
		cout << "REQ_MOVE" << endl;

		Vector3 position;
		memcpy(&position, buffer + sizeof(int) * 2 + sizeof(char) * ID_SIZE, sizeof(Vector3));
		cout << position.x << " | " << position.y << " | " << position.z << endl;

		p = new MovePacket(id, position);
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


int ServerAgent::checkHeader(char* buffer)
{
	cout << "deserialize" << endl;

	int h;
	memcpy(&h, buffer + sizeof(int), sizeof(int));
	cout << "[PacketHeader]::" << h << endl;
	return h;
}


// 스레드
DWORD WINAPI ServerAgent::SocketThread(LPVOID lpParam)
{
	CONParam* p = (CONParam*)lpParam;

	ServerAgent* This = p->sa;
	int i = p->cntNum;
	SOCKET sock = This->clients[i].sock;
	INT addressLen = This->clients[i].addrLen;

	INT retval;
	SOCKADDR_IN threadSocketAddress;

	//Get Client Information
	addressLen = sizeof(SOCKADDR_IN);
	getpeername(sock, (SOCKADDR*)&threadSocketAddress, &addressLen);

	CHAR		Buffer[BUFFER_SIZE];

	while (1)
	{
		//Receving Data
		memset(Buffer, 0, BUFFER_SIZE);

		retval = recv(sock, Buffer, BUFFER_SIZE, 0);
		if (retval == SOCKET_ERROR)
		{
			error_display(_T("recv()"));
			break;
		}
		else if (retval == 0)		// 소켓 종료
		{
			cout << "\n\n\n" << i << " 's client sock closed\n\n\n" << endl;
			break;
		}

		//Displaying Receiving Data
		int h = This->checkHeader(Buffer);
		if (h == P_ERROR) {
			cout << "HEADER: ERROR\n" << endl;
			continue;
		}

		//This->deserialize(sample, Buffer);
		switch (h) {
		case REQ_CON:
			cout << "HEADER: REQ_CON\n" << endl;
			This->send_ACKCON(i,  Buffer);
			break;
		case REQ_MOV:
			cout << "HEADER: REQ_MOVE\n" << endl;
			This->send_ACKMOV(i, Buffer);
			break;
		case CHATTING:
			cout << "HEADER: CHATTING\n" << endl;
			This->send_CHAT(i, Buffer);
			break;
		}
	}

	closesocket(sock);

	This->send_ACKCON_discon(i, Buffer);

	This->initializeCntInfo(i);
	std::wcout << _T("[TCP Server] Client Disconnected : IP Address=") 
		<< inet_ntoa(threadSocketAddress.sin_addr) << _T("PORT = ") << ntohs(threadSocketAddress.sin_port) << std::endl;

	return -1;
}


void ServerAgent::send_ACKCON(int i, char* buff) 
{
	cout << "\n\n--------CON START--------\n\n" << endl;

	// i번째를 제외한 모든 클라이언트에게 접속 메세지를 보냄
	memcpy(clients[i].id, buff + sizeof(int) * 2, sizeof(char) * ID_SIZE);

	char test[ID_SIZE + 1];
	memcpy(test, clients[i].id, ID_SIZE);
	test[ID_SIZE] = '\0';

	cout << clients[i].id[0] << clients[i].id[1] << clients[i].id[2] << clients[i].id[3] 
		<< clients[i].id[4] << clients[i].id[5] << clients[i].id[6] << endl;

	//bool con;
	//memcpy(&con, buffer + COMDATA_SIZE - 1, 1);
	//if (con == true)
	//	cout << "\n\n[client \'" << test << " \']: send ACK_CON\n\n" << endl;
	//else if (con == false)
	//	cout << "\n\n[client \'" << test << " \']: send ACK_CON___dis\n\n" << endl;

	ACK_CONPacket* pck = new ACK_CONPacket(clients[i].id , true);
	size_t pckSize = 0;

	int len = pck->get_length();
	if (len == -1) {
		cout << "PACKET GENARATION FAILED\n\n" << endl;
		return;
	}

	pck->serialize(buff, pckSize);

	DWORD dwWaitCode;

	for (int j = 0; j < CLIENT_MAX; j++) {
		if (j == i) continue;
		if (clients[j].sock == NULL) continue;

		dwWaitCode = WaitForSingleObject(hMutex, INFINITE);
		if (dwWaitCode == WAIT_FAILED)
		{
			cout << "WaitForSingleObject failed : " << GetLastError() << endl;
			return;
		}
		if (dwWaitCode == WAIT_ABANDONED)
			cout << "Abandoned Mutex acquired!!!" << GetLastError() << endl;

		send(clients[j].sock, buff, pckSize, 0);

		ReleaseMutex(hMutex);
#if TEST==0
		cout << "\n\n[client" << j << "]: send ACK_CON\n\n" << endl;
#endif
	}

	pck->~ACK_CONPacket();
	pck = nullptr;

	cout << "\n\n--------CON FINISH--------\n\n" << endl;
}

void ServerAgent::send_ACKMOV(int i, char* buff)
{
	// i번째를 제외한 모든 클라이언트에게 접속 메세지를 보냄
	// deserialize(sample, buff);

	//MovePacket* s_pck = dynamic_cast<MovePacket*>(sample);
	//if (s_pck == NULL) return;

	cout << "REQ_CHAT" << endl;

	Vector3 position;
	memcpy(&position, buff + sizeof(int) * 2 + sizeof(char) * ID_SIZE, sizeof(Vector3));

	ACK_MovePacket* pck = new ACK_MovePacket(clients[i].id, position, inet_ntoa(clients[i].addr.sin_addr));
	size_t pckSize = 0;

	int len = pck->get_length();
	if (len == -1) {
		cout << "PACKET GENARATION FAILED\n\n" << endl;
		return;
	}

	pck->serialize(buff, pckSize);

	DWORD dwWaitCode;

	for (int j = 0; j < CLIENT_MAX; j++) {
		if (j == i) continue;
		if (clients[j].sock == NULL) continue;

		dwWaitCode = WaitForSingleObject(hMutex, INFINITE);
		if (dwWaitCode == WAIT_FAILED)
		{
			cout << "WaitForSingleObject failed : " << GetLastError() << endl;
			return;
		}
		if (dwWaitCode == WAIT_ABANDONED)
			cout << "Abandoned Mutex acquired!!!" << GetLastError() << endl;

		send(clients[j].sock, buff, pckSize, 0);

		ReleaseMutex(hMutex);
#if TEST==0
		cout << "\n\n[client" << j << "]: send ACK_CON\n\n" << endl;
#endif
	}

	pck->~ACK_MovePacket();
	pck = nullptr;
}

void ServerAgent::send_CHAT(int i, char* buff) 
{
	cout << "\n\n--------CHATTING--------\n\n" << endl;

	// for문 안에서 clientid를 넘겨주면 오류 발생. 왜일까?
	char thisid[ID_SIZE];
	memcpy(thisid, clients[i].id, ID_SIZE);

	char otherid[ID_SIZE];
	memcpy(otherid, buff + 8 + ID_SIZE, ID_SIZE);

	char msg_c[MSG_SIZE];
	memcpy(&msg_c, buff + 8 + 2 * ID_SIZE, MSG_SIZE);

	cout << msg_c << endl;

	DWORD dwWaitCode;

	bool w = false;
	for (int j = 0; j < CLIENT_MAX; j++) {
		if (j == i) continue;
		if (clients[j].sock == NULL) continue;

#if TEST==0
		cout << "\n\n[client" << j << "]: send ACK_CON\n\n" << endl;
#endif

		for (int k = 0; i < ID_SIZE; i++) {
			if (clients[j].id[k] != otherid[k]) {
				cout << "\n\n[" << otherid[k] << "]: wrong\n\n" << endl;  
				w = true;	
				break;
			}
		}
		if (w == true) continue;

		string str(msg_c);

		ChatPacket* pck = new ChatPacket(otherid, thisid, str);
		size_t pckSize = 0;

		int len = pck->get_length();
		if (len == -1) {
			cout << "PACKET GENARATION FAILED\n\n" << endl;
			return;
		}

		pck->serialize(buff, pckSize);

		dwWaitCode = WaitForSingleObject(hMutex, INFINITE);
		if (dwWaitCode == WAIT_FAILED)
		{
			cout << "WaitForSingleObject failed : " << GetLastError() << endl;
			return;
		}
		if (dwWaitCode == WAIT_ABANDONED)
			cout << "Abandoned Mutex acquired!!!" << GetLastError() << endl;

		send(clients[j].sock, buff, pckSize, 0);

		ReleaseMutex(hMutex);
#if TEST==0
		cout << "\n\n[client" << j << "]: send ACK_CON\n\n" << endl;
#endif

		pck->~ChatPacket();
		pck = nullptr;

		return;
	}

	cout << "\n\nERROR:: NON-ID" << endl;

	// 해당하는 클라이언트 id 없음 
	ChatPacket* pck = new ChatPacket(otherid, clients[i].id, "ERROR:: NON-ID");
	size_t pckSize = 0;

	int len = pck->get_length();
	if (len == -1) {
		cout << "PACKET GENARATION FAILED\n\n" << endl;
		return;
	}

	pck->serialize(buff, pckSize);

	send(clients[i].sock, buff, pckSize, 0);

	pck->~ChatPacket();
	pck = nullptr;
}

void ServerAgent::send_ACKCON_discon(int i, char* buff)
{
	cout << "\n\n--------DISCON START--------\n\n" << endl;

	ACK_CONPacket* pck = new ACK_CONPacket(clients[i].id, false);
	size_t pckSize = 0;
	
	int len = pck->get_length();
	if (len == -1) {
		cout << "PACKET GENARATION FAILED\n\n" << endl;
		return;
	}

	pck->serialize(buff, pckSize);

	DWORD dwWaitCode;

	for (int j = 0; j < CLIENT_MAX; j++) {
		if (j == i) continue;
		if (clients[j].sock == NULL) continue;

		dwWaitCode = WaitForSingleObject(hMutex, INFINITE);
		if (dwWaitCode == WAIT_FAILED)
		{
			cout << "WaitForSingleObject failed : " << GetLastError() << endl;
			return;
		}
		if (dwWaitCode == WAIT_ABANDONED)
			cout << "Abandoned Mutex acquired!!!" << GetLastError() << endl;

		send(clients[j].sock, buff, pckSize, 0);

		ReleaseMutex(hMutex);
#if TEST==0
		cout << "\n\n[client" << j << "]: send ACK_CON____discon\n\n" << endl;
#endif
	}

	pck->~ACK_CONPacket();
	pck = nullptr;

	cout << "\n\n--------DISCON FINISH--------\n\n" << endl;
}


