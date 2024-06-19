#include "ServerAgent.h"


int main(int argc, char* argv[])
{
	ServerAgent* server;

	server = new ServerAgent();
	server->setReadyState();

	server->communicate();


	return 0;
}

