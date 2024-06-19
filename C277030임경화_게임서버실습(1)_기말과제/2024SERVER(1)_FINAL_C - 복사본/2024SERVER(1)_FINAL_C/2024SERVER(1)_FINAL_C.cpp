#include "ClientAgent.h"


int main(int argc, char* argv[])
{
	/*
	ClientAgent* clients[CLIENT_MAX];

	for (int i = 0; i < CLIENT_MAX; i++)
	{
		clients[i] = new ClientAgent(i);
		clients[i]->setReadyState();
	}

	for (int i = 0; i < CLIENT_MAX; i++)
		clients[i]->communicate();

	return 0;
	*/

	srand(time(NULL));


	ClientAgent* client;

	client = new ClientAgent();
	client->setReadyState();

	client->communicate();

	return 0;
}

