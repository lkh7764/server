#pragma once																		// ��� �ߺ� ����
#pragma comment(lib, "ws2_32")									// ���� ���̺귯��
#define _WINSOCK_DEPRECATED_NO_WARNINGS		// ���� ���� ����
#define _CRT_SECURE_NO_WARNINGS							// scanf ���� ����

#include <WinSock2.h>
#include <stdlib.h>
#include <iostream>
#include <tchar.h>
using namespace std;

#define BUFFERSIZE 		20
#define DATASIZE				20
#define SERVER_PORT	9000
#define SERVER_IP			"192.168.45.186"
#define CLIENTS_NUM	10


// ���� ��� �� ���α׷� ����
VOID error_Quit(const wchar_t* msg);

// ���� ���
VOID error_Display(const wchar_t* msg);

