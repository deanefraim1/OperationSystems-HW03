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

int Helpers::InitializeSocket()
{
    int sockedFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockedFd < 0)
    {
        cout << "Error: socket() failed" << endl;
        exit(1);
    }
    return sockedFd;
}

void Helpers::BindSocket(Session session)
{
    if (bind(session.socketFd, (struct sockaddr *)&(session.serverAddress), sizeof(session.serverAddress)) < 0)
    {
        cout << "Error: bind() failed" << endl;
        exit(1);
    }
}

void Helpers::RecieveMassage(Session session)
{
    string massage;
    fd_set socketFdSet;
    struct timeval timeoutVal;
    struct WRQ wrqPacket;
    int fileToWriteTo;
    sockaddr_in originalClientAddress;
    while (1)
    {
        FD_ZERO(&socketFdSet);
        FD_SET(session.socketFd, &socketFdSet);

        timeoutVal.tv_sec = session.timeout;
        timeoutVal.tv_usec = 0;

        int selectReturnValue = select(session.socketFd + 1, &socketFdSet, NULL, NULL, &timeoutVal);
        if(selectReturnValue < 0)
        {
            cout << "Error: select() failed" << endl;
            exit(1);
        }
        else if(selectReturnValue == 0)
        {
            cout << "Error: select() timed out" << endl;
            if(fileToWriteTo > 0)
                remove(wrqPacket.filename.c_str());

            break;
        }
        else 
        {
            int bytesRecieved = recvfrom(session.socketFd, session.buffer, session.maxBufferSize, 0, (struct sockaddr *)&(session.clientAddress), &(session.clientAddressLength));
            if (bytesRecieved < 0)
            {
                cout << "Error: recvfrom() failed" << endl;
                exit(1);
            }
            if(session.buffer[0] == '2')
            {
                if(wrqPacket != NULL)
                {
                    cout << "Error: received more than one WRQ packet" << endl;
                }
                else
                {
                    originalClientAddress = session.clientAddress;
                    wrqPacket = ParseBufferAsWrqPacket(session.buffer);
                    fileToWriteTo = open(wrqPacket.filename.c_str(), O_WRONLY | O_CREAT, 0666);
                    if(fileToWriteTo < 0)
                    {
                        cout << "Error: open() failed" << endl;
                        exit(1);
                    }
                    struct ACK ackPacket;
                    ackPacket.blockNumber = '0';
                    int bytesSent = sendto(session.socketFd, &ackPacket, sizeof(ackPacket), 0, (struct sockaddr *)&(session.clientAddress), session.clientAddressLength);
                    if(bytesSent < 0)
                    {
                        cout << "Error: sendto() failed" << endl;
                        exit(1);
                    }
                }
            }
            else if(session.buffer[0] == '3')
            {
                if(session.clientAddress != originalClientAddress)
                {
                    cout << "Error: received DATA packet from wrong client" << endl;
                    break;
                }
                else
                {
                    struct Data dataPacket = ParseBufferAsDataPacket(session.buffer);
                    int bytesWritten = write(fileToWriteTo, dataPacket.data, strlen(dataPacket.data));
                    if(bytesWritten < 0)
                    {
                        cout << "Error: write() failed" << endl;
                        exit(1);
                    }
                    else if(bytesWritten < strlen(dataPacket.data))
                    {
                        int fileCloseReturnValue = close(fileToWriteTo);
                        if (fileCloseReturnValue < 0)
                        {
                            cout << "Error: close() failed" << endl;
                            exit(1);
                        }
                        break;
                    }
                }
            }
        }
    }
}

struct WRQ Helpers::ParseBufferAsWrqPacket(char buffer[516])
{
    struct WRQ wrqPacket;
    wrqPacket.filename = string(buffer + 2);
    wrqPacket.transmissionMode = string(buffer + 2 + wrqPacket.filename.length() + 1);
    if(wrqPacket.transmissionMode != "octet")
    {
        cout << "Error: transmission mode is not octet" << endl;
        exit(1);
    }
    return wrqPacket;
}

struct Data Helpers::ParseBufferAsDataPacket(char buffer[516])
{
    struct Data dataPacket;
    dataPacket.blockNumber[0] = buffer[2];
    dataPacket.blockNumber[1] = buffer[3];
    for(int i = 4; i < 516; i++)
    {
        dataPacket.data[i - 4] = buffer[i];
        
        if(buffer[i] == '\0')
            break;
    }
    return dataPacket;
}