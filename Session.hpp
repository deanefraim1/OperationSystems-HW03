#ifndef SESSION_HPP
#define SESSION_HPP

#include <netinet/in.h>
#include <sys/time.h>
#include <string>

using namespace std;

#define MAX_BUFFER_SIZE 512

class Session
{
public:
    int socketFd;
    int fileToWriteToFd;
    string fileToWriteToName;
    struct sockaddr_in serverAddress;
    socklen_t serverAddressLength;
    struct sockaddr_in originalClientAddress;
    socklen_t originalClientAddressLength;
    struct sockaddr_in currentClientAddress;
    socklen_t currentClientAddressLength;
    char dataBuffer[MAX_BUFFER_SIZE];
    const int maxDataBufferSize = MAX_BUFFER_SIZE;
    timeval timeoutLimitVal;
    int maxNumberOfResendsAllowed;
    int currentNumberOfResends;
    short numberOfBlocksRecieved;

    Session(unsigned short serverPort, int timeout, int maxNumberOfResendsAllowed);
    void InitializeSocket();
    void EndClientConnection();
    void SendAckPacket();
    void SendErrorPacket(short errorCode, string errorMessage);
    int RecievePacketFromClient();
    void HandleWrqPacket();
    int HandleDataPacket();
};

#endif