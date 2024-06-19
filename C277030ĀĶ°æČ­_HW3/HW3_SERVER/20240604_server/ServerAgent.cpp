#include "ServerAgent.h"



// 통신 준비: 소켓 생성 > 바인드 > 리슨
VOID ServerAgent::setReadyState()
{
	INT retval;

	// socket()
	listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSock == INVALID_SOCKET)
		error_Quit(_T("socket()"));

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
		error_Quit(_T("bind()"));

	// listen()
	retval = listen(listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		error_Quit(_T("listen()"));
}

// 통신: 어셉트 > SocketThread() 실행
VOID ServerAgent::communicate()
{
	HANDLE hThread;

	while (1)
	{
		//Accept()
		addrLen = sizeof(clientAddr);
		clientSock = accept(listenSock , (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSock == INVALID_SOCKET)
		{
			error_Display(_T("Accept"));
			break;
		}

		//Displaying Client Display
		std::wcout << std::endl << _T("[TCP Server] Client Connected : IP Address = ") 
			<< inet_ntoa(clientAddr.sin_addr) << _T(", Port = ") << ntohs(clientAddr.sin_port) << std::endl;

		//Create Thread
		hThread = CreateThread(NULL, 0, SocketThread, this, 0, NULL);

		SetEvent(hEvent);

		WaitForSingleObject(hThread, INFINITE);

		if (hThread == NULL)
			closesocket(clientSock);
		else
			CloseHandle(hThread);
	}
}

// 스레드
DWORD WINAPI ServerAgent::SocketThread(LPVOID lpParam)
{
	ServerAgent* This = (ServerAgent*)lpParam;
	SOCKET sock = This->clientSock;
	INT retval;
	INT addressLen;
	SOCKADDR_IN threadSocketAddress;
	CHAR Buffer[BUFFERSIZE];

	//Get Client Information
	addressLen = sizeof(SOCKADDR_IN);
	getpeername(sock, (SOCKADDR*)&threadSocketAddress, &addressLen);

	while (1)
	{
		//Receving Data
		retval = recv(sock, Buffer, BUFFERSIZE, 0);
		if (retval == SOCKET_ERROR)
		{
			error_Display(_T("recv()"));
			break;
		}
		else if (retval == 0)
			break;

		//Displaying Receiving Data
		Buffer[retval] = _T('\0');
		std::wcout << _T("[TCP/") << inet_ntoa(threadSocketAddress.sin_addr) 
			<< _T(":") << ntohs(threadSocketAddress.sin_port) << _T(" \" ") << Buffer << _T(" \" ") << std::endl;
	}
	closesocket(sock);
	std::wcout << _T("[TCP Server] Client Disconnected : IP Address=") << inet_ntoa(threadSocketAddress.sin_addr) << _T("PORT = ") << ntohs(threadSocketAddress.sin_port) << std::endl;

	return 1;
}