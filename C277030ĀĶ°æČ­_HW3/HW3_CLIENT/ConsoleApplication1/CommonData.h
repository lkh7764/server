#pragma once																		// 헤더 중복 방지
#pragma comment(lib, "ws2_32")									// 윈속 라이브러리
#define _WINSOCK_DEPRECATED_NO_WARNINGS		// 윈속 오류 방지
#define _CRT_SECURE_NO_WARNINGS							// scanf 오류 방지

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


// 에러 출력 후 프로그램 종료
VOID error_Quit(const wchar_t* msg);

// 에러 출력
VOID error_Display(const wchar_t* msg);

