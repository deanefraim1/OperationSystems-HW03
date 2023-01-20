#ifndef SESSION_HPP
#define SESSION_HPP

#include <netinet/in.h>
#include <sys/time.h>
#include <string>
#include "Address.hpp"
#include "FileManager.hpp"

#define END_CONNECTION_SUCCESS 1
#define END_CONNECTION_FAILURE -1
#define GET_NEXT_PACKET 0
#define NOT_EXIST -1
#define EMPTY -1
#define MAX_BUFFER_SIZE 516

using namespace std;

class Session
{
public:
    int socketFd;
    FileManager originalClientFileToWriteTo;
    Address serverAddress;
    Address originalClientAddress;
    Address currentPacketClientAddress;
    char packetDataBuffer[MAX_BUFFER_SIZE];
    timeval timeoutLimitVal;
    int maxNumberOfResendsAllowed;
    int currentNumberOfResends;
    short numberOfBlocksRecieved;

    Session(unsigned short serverPort, int timeout, int maxNumberOfResendsAllowed);
    void InitializeSocket();
    void InitializeServerAddress(unsigned short serverPort);
    void EndClientConnection();
    void CleanOriginalClientAddress();
    void CleanCurrentPacketClientAddress();
    void CleanDataBuffer();
    void SendAckPacket();
    void SendErrorPacketToCurrentPacketClient(short errorCode, string errorMessage);
    void SendErrorPacketToOriginalClient(short errorCode, string errorMessage);
    int RecievePacketFromClient();
    int HandleWrqPacket();
    int HandleDataPacket();
};

#endif