/*
			------------------------------------------------------------------------------------
			CommonData.h
			------------------------------------------------------------------------------------
			모든 클래스+소스코드가 공통으로 가져야 하는 데이터
				> 명령어
				> 헤더파일
				> 서버 및 기본 정보 디파인
				> 오류 제어 함수
				> struct Vector3
*/

#pragma once
#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <stdlib.h>
#include <tchar.h>
#include <ctime>
#include <string>
#include <cstring>

#include <iostream>
using namespace std;

#define BUFFER_SIZE		512
#define MSG_SIZE			128
#define SERVER_PORT	9000
#define SERVER_IP			"192.168.45.186"	//CMD > IPCONFIG
#define CLIENT_MAX		10

#define ID_SIZE					7
#define IDFLAG					'$'
#define ENDFLAG				'#'
#define COMDATA_SIZE	9+ID_SIZE
#define IPADDR_SIZE		16

#define TEST						1


void error_quit(const wchar_t* msg);
void error_display(const wchar_t* msg);

bool checkID(char* buf, char id[ID_SIZE]);


typedef struct {
	float x;
	float y;
	float z;
}Vector3;