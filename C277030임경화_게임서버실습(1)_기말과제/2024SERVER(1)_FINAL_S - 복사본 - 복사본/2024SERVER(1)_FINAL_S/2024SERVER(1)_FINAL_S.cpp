#include "ServerAgent.h"


int main(int argc, char* argv[])
{
    //CONPacket* test = new CONPacket("C277030");
    //char testBuff[BUFFERSIZE] = { 0 };
    //size_t testSize;
    //test->serialize(testBuff, testSize);

    //int len = -1;
    //memcpy(&len, testBuff, 4);
    //int hea = -1;
    //memcpy(&hea, testBuff+4, 4);

    //cout << len << endl;
    //cout << hea << endl;

    //std::cout << "Hello World!\n";

    ServerAgent* server;

    server = new ServerAgent();
    server->setReadyState();

    server->communicate();


    return 0;

}
