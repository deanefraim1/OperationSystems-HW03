#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include "Helpers.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include "Session.hpp"
#include "PacketStructs.hpp"
#include <fcntl.h>
#include <unistd.h>

using namespace std;

void Helpers::BindAddressToSocket(struct sockaddr_in address, int socketFd)
{
    int bindReturnValue = ::bind(socketFd, (struct sockaddr *)&address, sizeof(address));
    if (bindReturnValue < 0)
    {
        cout << "Error: bind() failed" << endl;
        exit(1);
    }
}

void Helpers::ReceiveMassage(Session session)
{
    while (1)
    {
        fd_set socketFdSet = Helpers::GetSocketFdSetFromOneSocketFd(session.socketFd);

        int selectReturnValue = select(session.socketFd + 1, &socketFdSet, NULL, NULL, &(session.timeoutLimitVal));
        if(selectReturnValue < 0)
            Helpers::ExitProgramWithPERROR("select() failed");

        else if(selectReturnValue == 0) // timeout has reached
        {
            if(session.currentNumberOfResends >= session.maxNumberOfResendsAllowed)
            {
                if(session.fileToWriteToFd > 0)
                    remove(session.fileToWriteToName.c_str());

                session.EndClientConnection();
            }
            else
                session.SendAckPacket();
        }
        else 
            session.RecievePacketFromClient();
    }
}

struct WrqPacket Helpers::ParseBufferAsWrqPacket(char buffer[516])
{
    struct WrqPacket wrqPacket;
    wrqPacket.fileName = string(buffer + 2);
    wrqPacket.transmissionMode = string(buffer + 2 + wrqPacket.fileName.length() + 1);
    if(wrqPacket.transmissionMode != "octet")
    {
        cout << "Error: transmission mode is not octet" << endl;
        exit(1);
    }
    return wrqPacket;
}

struct DataPacket Helpers::ParseBufferAsDataPacket(char buffer[516])
{
    struct DataPacket dataPacket;
    //TODO - insert the block number from buffer to dataPacket  
    for (int i = 4; i < 516; i++)
    {
        dataPacket.data[i - 4] = buffer[i];
        
        if(buffer[i] == '\0')
            break;
    }
    return dataPacket;
}

struct AckPacket Helpers::ParseBufferAsAckPacket(char buffer[516])
{
    struct AckPacket ackPacket;
    //TODO - insert the block number from buffer to dataPacket  
    return ackPacket;
}

fd_set Helpers::GetSocketFdSetFromOneSocketFd(int socketFd)
{
    fd_set socketFdSet;
    FD_ZERO(&socketFdSet);
    FD_SET(socketFd, &socketFdSet);
    return socketFdSet;
}

struct timeval Helpers::ParseTimeoutLimitAsTimeval(int timeoutLimit)
{
    struct timeval timeoutVal;
    timeoutVal.tv_sec = timeoutLimit;
    timeoutVal.tv_usec = 0;
    return timeoutVal;
}

void Helpers::ExitProgramWithPERROR(string errormessage)
{
    perror(("TTFTP_ERROR:" + errormessage).c_str());
    exit(1);
}

void Helpers::ExitProgramWithSTDERROR(string errorMessage)
{
    cout << "TTFTP_ERROR:" << errorMessage << endl;
    exit(1);
}