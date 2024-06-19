#include "Packet.h"


// Packet
Packet::Packet()
	: length(0), header(REQ_CON), endFlag(ENDFLAG)
{
	clientID[0] = NULL;
}

Packet::~Packet()
{
	length = 0;
	header = REQ_CON;
	clientID[0] = NULL;
	endFlag = NULL;
}

const int Packet::get_length() { return length; }
//const PacketHeader Packet::get_header() { return header; }
const char* Packet::get_clientID() { return clientID; }
//const char Packet::get_endFlag() { return endFlag; }


// CONPacket
CONPacket::CONPacket(bool con) : Packet(), connection(con) {}

CONPacket::CONPacket(char id[ID_SIZE], bool con) : Packet()//, connection(con)
{
	if (checkID(clientID, id) == false) {
#if TEST==0
		cout << "CONPacket generation failed" << endl;
#endif
		length = -1;
		return;
	}
#if TEST==0
	cout << "CONPacket generation success\n ID:" << id << endl;
#endif

	connection = con;
	header = REQ_CON;
}

CONPacket::~CONPacket()
{
	connection = false;
}

void CONPacket::serialize(char* buffer, size_t& packet_size)
{
#if TEST==0
	cout << "\n\n-------------CONPACK SERIALIZE START-------------\n" << endl;
#endif

	//memset(buffer, 0, BUFFER_SIZE);

	// length
	length = COMDATA_SIZE + sizeof(bool);
	memcpy(buffer, &length, sizeof(int));
	// header
	memcpy(buffer + 4, &header, sizeof(int));
	// clientID
	memcpy(buffer + 8, &clientID, ID_SIZE);
	// connection
	memcpy(buffer + COMDATA_SIZE - 1, &connection, sizeof(bool));
	// endflag
	buffer[length - 1] = endFlag;

	packet_size = length;

#if TEST==0
	cout << "\n-------------CONPACK SERIALIZE FINISH-------------\n\n" << endl;
#endif
}

const bool CONPacket::IsConnectionPacket() { return connection; }


// ACK_CONPacket
ACK_CONPacket::ACK_CONPacket(char id[ID_SIZE], bool con) : CONPacket(con)
{
	if (checkID(clientID, id) == false) {
#if TEST==0
		cout << "ACK_CONPacket generation failed" << endl;
#endif
		length = -1;
		return;
	}
#if TEST==0
	cout << "ACK_CONPacket generation success\n ID:" << id << endl;
#endif

	// conTime
	time_t t;
	struct tm* tp;
	t = time(NULL);
	tp = localtime(&t);

	conTime[0] = tp->tm_year + 1900;	// 년도
	conTime[1] = tp->tm_mon + 1;			// 월
	conTime[2] = tp->tm_mday;				// 일
	conTime[3] = tp->tm_hour;				// 시
	conTime[4] = tp->tm_min;					// 분
	conTime[5] = tp->tm_sec;					// 초

	// header
	header = ACK_CON;
}

ACK_CONPacket::ACK_CONPacket(char id[ID_SIZE], bool con, int ct[6]) : CONPacket(con)
{
	if (checkID(clientID, id) == false) {
#if TEST==0
		cout << "ACK_CONPacket generation failed" << endl;
#endif
		length = -1;
		return;
	}
#if TEST==0
	cout << "ACK_CONPacket generation success\n ID:" << id << endl;
#endif

	// conTime
	for (int i = 0; i < 6; i++)
		conTime[i] = ct[i];

	// header
	header = ACK_CON;
}

ACK_CONPacket::~ACK_CONPacket()
{
	memset(conTime, 0, 6);
}

void ACK_CONPacket::serialize(char* buffer, size_t& packet_size)
{
#if TEST==0
	cout << "\n\n-------------ACK_CONPACK SERIALIZE START-------------\n" << endl;
#endif

	//memset(buffer, 0, BUFFER_SIZE);

	// length
	length = COMDATA_SIZE + 1 + (4 * 6);
	memcpy(buffer, &length, 4);
	// header
	memcpy(buffer + 4, &header, 4);
	// clientID
	memcpy(buffer + 8, &clientID, ID_SIZE);
	// connection
	memcpy(buffer + COMDATA_SIZE - 1, &connection, 1);
	// conTime
	memcpy(buffer + COMDATA_SIZE, conTime, 4 * 6);
	// endflag
	buffer[length - 1] = endFlag;

	packet_size = length;

#if TEST==0
	cout << "\n-------------ACK_CONPACK SERIALIZE FINISH-------------\n\n" << endl;
#endif
}



MovePacket::MovePacket(char id[ID_SIZE], Vector3 p) : Packet()
{
	if (checkID(clientID, id) == false) {
#if TEST==0
		cout << "MovePacket generation failed" << endl;
#endif
		length = -1;
		return;
	}
#if TEST==0
	cout << "MovePacket generation success\n ID:" << id << endl;
#endif

	pos = p;

	header = REQ_MOV;
}

MovePacket::~MovePacket()
{
}

void MovePacket::serialize(char* buffer, size_t& packet_size)
{
#if TEST==0
	cout << "\n\n-------------MOVPACK SERIALIZE START-------------\n" << endl;
#endif

	//memset(buffer, 0, BUFFER_SIZE);

	// length
	length = COMDATA_SIZE + sizeof(Vector3);
	memcpy(buffer, &length, 4);
	// header
	memcpy(buffer + 4, &header, 4);
	// clientID
	memcpy(buffer + 8, &clientID, ID_SIZE);
	// pos
	memcpy(buffer + COMDATA_SIZE - 1, &pos, sizeof(Vector3));
	// endflag
	buffer[length - 1] = endFlag;

	packet_size = length;

#if TEST==0
	cout << "\n-------------MOVPACK SERIALIZE FINISH-------------\n\n" << endl;
#endif
}



ACK_MovePacket::ACK_MovePacket(char id[ID_SIZE], Vector3 p, string ipAddr)
	: MovePacket(id, p)
{
	strcpy(IPAddr, ipAddr.c_str());

	cout << "ACK_MovePacket generation success:: [IP Address] " << IPAddr << endl;

	header = ACK_MOV;
}

ACK_MovePacket::~ACK_MovePacket() {}

void ACK_MovePacket::serialize(char* buffer, size_t& packet_size)
{
#if TEST==0
	cout << "\n\n-------------ACK_MOVPACK SERIALIZE START-------------\n" << endl;
#endif

	//memset(buffer, 0, BUFFER_SIZE);

	// length
	length = COMDATA_SIZE + sizeof(Vector3) + IPADDR_SIZE;
	memcpy(buffer, &length, 4);
	// header
	memcpy(buffer + 4, &header, 4);
	// clientID
	memcpy(buffer + 8, &clientID, ID_SIZE);
	// pos
	memcpy(buffer + COMDATA_SIZE - 1, &pos, sizeof(Vector3));
	// IPAddr;
	memcpy(buffer + COMDATA_SIZE - 1 + sizeof(Vector3), IPAddr, IPADDR_SIZE);
	// endflag
	buffer[length - 1] = endFlag;

	packet_size = length;

#if TEST==0
	cout << "\n-------------ACK_MOVPACK SERIALIZE FINISH-------------\n\n" << endl;
#endif
}



ChatPacket::ChatPacket(char id[ID_SIZE], char otherid[ID_SIZE], string msg_s) : Packet()
{
	if (checkID(clientID, id) == false) {
#if TEST==0
		cout << "CHATPK:: thisID generation failed" << endl;
#endif
		length = -1;
		return;
	}
#if TEST==0
	cout << "CHATPK:: thisID generation success\n ID:" << id << endl;
#endif

	if (checkID(otherID, otherid) == false) {
#if TEST==0
		cout << "CHATPK:: otherID generation failed" << endl;
#endif
		length = -1;
		return;
	}
#if TEST==0
	cout << "CHATPK:: otherID generation success\n ID:" << otherid << endl;
#endif

	strcpy(msg, msg_s.c_str());
	header = CHATTING;
}

ChatPacket::~ChatPacket()
{
	otherID[0] = NULL;
	msg[0] = NULL;
}

void ChatPacket::serialize(char* buffer, size_t& packet_size)
{
#if TEST==0
	cout << "\n\n-------------CHATTING SERIALIZE START-------------\n" << endl;
#endif

	//memset(buffer, 0, BUFFER_SIZE);

	// length
	length = COMDATA_SIZE + ID_SIZE + sizeof(msg);
	memcpy(buffer, &length, 4);
	// header
	memcpy(buffer + 4, &header, 4);
	// clientID
	memcpy(buffer + 8, &clientID, ID_SIZE);
	// otherID
	memcpy(buffer + 8 + ID_SIZE, &otherID, ID_SIZE);
	// msg
	memcpy(buffer + 8 + 2 * ID_SIZE, &msg, sizeof(msg));
	// endflag
	buffer[length - 1] = endFlag;

	packet_size = length;

#if TEST==0
	cout << "\n-------------CHATTING SERIALIZE FINISH-------------\n\n" << endl;
#endif
}


IDInfoPacket::IDInfoPacket(char* ids, int len) : Packet()
{
	flag = IDFLAG;

	strcpy(IDs, ids);
}

IDInfoPacket::~IDInfoPacket()
{
	IDs[0] = NULL;
	flag = NULL;
}

void IDInfoPacket::serialize(char* buffer, size_t& packet_size)
{
	// length
	length = COMDATA_SIZE + (ID_SIZE + 1) * (CLIENT_MAX - 1) + 1;
	memcpy(buffer, &length, 4);
	// header
	memcpy(buffer + 4, &header, 4);
	// clientID
	memcpy(buffer + 8, &clientID, ID_SIZE);
	// flag
	memcpy(buffer + COMDATA_SIZE - 1, &flag, 1);
	// ids
	memcpy(buffer + COMDATA_SIZE, IDs, (ID_SIZE + 1) * (CLIENT_MAX - 1));
	// endflag
	buffer[length - 1] = endFlag;

	packet_size = length;
}
