/*
			------------------------------------------------------------------------------------
			Packet.h
			------------------------------------------------------------------------------------
			패킷 관련 데이터
				> 패킷 헤더 enum
				> 모든 패킷 클래스
*/

#pragma once
#include "CommonData.h"



//enum PacketHeader {
//	PK_ERROR = -1,		
//	REQ_CON = 0,		// C to S, connect&disconnect
//	ACK_CON = 1,		// S to C, connect&disconnect
//	REQ_MOVE = 2,	// C to S, clientMove
//	ACK_MOVE = 3,	// S to C, clientMove
//	CHATTING = 4		// C to S & S to C, chatting
//};

#define P_ERROR			-1
#define REQ_CON		1
#define ACK_CON		2
#define REQ_MOV		3
#define ACK_MOV		4
#define CHATTING		5
#define ID_INFO			6


class Packet
{
protected:
	int						length;
	int						header;
	char						clientID[ID_SIZE];
	char						endFlag;

	Packet();
	~Packet();

public:
	const int							get_length();
	//const PacketHeader	get_header();
	const char*					get_clientID();
	//const char					get_endFlag();

	virtual void serialize(char* buffer, size_t& packet_size) = 0;
};


class CONPacket : public Packet
{
	// PACKET HEADER: REQ_CON
protected:
	bool connection;

public:
	CONPacket(bool con);										// ACK_CONPacket 실행
	CONPacket(char id[ID_SIZE], bool con);		// REQ_CONPacket 실행
	~CONPacket();

	const bool IsConnectionPacket();
	void serialize(char* buffer, size_t& packet_size) override;
};

class ACK_CONPacket : public CONPacket
{
	// PACKET HEADER: ACK_CON
private:
	int conTime[6];

public:
	ACK_CONPacket(char id[ID_SIZE], bool con);
	ACK_CONPacket(char id[ID_SIZE], bool con, int ct[6]);
	~ACK_CONPacket();

	//const time_t get_time();
	virtual void serialize(char* buffer, size_t& packet_size) override;
};


class MovePacket : public Packet
{
	// PACKET HEADER: REQ_MOVE
protected:
	Vector3 pos;

public:
	MovePacket(char id[ID_SIZE], Vector3 p);
	~MovePacket();

	//const Vector3 get_pos();
	virtual void serialize(char* buffer, size_t& packet_size) override;
};

class ACK_MovePacket : public MovePacket
{
	// PACKET HEADER: ACK_MOVE
private:
	char IPAddr[IPADDR_SIZE];

public:
	ACK_MovePacket(char id[ID_SIZE], Vector3 p, string ipAddr);
	~ACK_MovePacket();

	//const char* get_IPAddr();
	virtual void serialize(char* buffer, size_t& packet_size) override;
};


class ChatPacket : public Packet
{
	// PACKET HEADER: CHATTING
private:
	char		otherID[ID_SIZE];
	char		msg[MSG_SIZE];

public:
	ChatPacket(char id[ID_SIZE], char otherid[ID_SIZE], string msg_s);
	~ChatPacket();

	//const char* get_otherID();
	//const char* get_msg();
	virtual void serialize(char* buffer, size_t& packet_size) override;
};


class IDInfoPacket : public Packet
{
private:
	char		IDs[(ID_SIZE + 1) * (CLIENT_MAX - 1)];
	char		flag;

public:
	IDInfoPacket(char* ids, int len);
	~IDInfoPacket();

	virtual void serialize(char* buffer, size_t& packet_size) override;
};

