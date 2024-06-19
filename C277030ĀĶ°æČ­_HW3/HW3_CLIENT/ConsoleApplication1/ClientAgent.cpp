#include "ClientAgent.h"
#include <string>


VOID ClientAgent::setReadyState()
{
	// socket()
	connectSock = socket(AF_INET, SOCK_STREAM, 0);
	if (connectSock == INVALID_SOCKET)
		error_Quit(_T("socket()"));
}

VOID ClientAgent::communicate()
{
	INT			retval;
	//HANDLE hThread;

	// connect()
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	serverAddr.sin_port = htons(SERVER_PORT);

	retval = connect(connectSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR)
		error_Quit(_T("connect()"));

	string test = "Client send ";
	test += to_string(primeNum);
	strcpy(buffer, test.c_str());
	std::wcout << buffer << std::endl;
	send(connectSock, &buffer[0], strlen(buffer), 0);

	closesocket(connectSock);
}