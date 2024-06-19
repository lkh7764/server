#include "ClientAgent.h"


int main(int argc, char* argv[])
{
	ClientAgent* clients[CLIENTS_NUM];

	for (int i = 0; i < CLIENTS_NUM; i++)
	{
		clients[i] = new ClientAgent(i);
		clients[i]->setReadyState();
	}

	for (int i = 0; i < CLIENTS_NUM; i++)
		clients[i]->communicate();

	return 0;

	//ClientAgent* client;

	//client = new ClientAgent();
	//client->setReadyState();

	//client->communicate();

	//return 0;
}

