#include "Session.hpp"
#include "Helpers.hpp"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include "PacketStructs.hpp"
#include <fcntl.h>

#define END_CONNECTION 1
#define GET_NEXT_PACKET 0


using namespace std;

Session::Session(unsigned short serverPort, int timeoutLimit, int maxNumberOfResendsAllowed)
{
    this->InitializeSocket();

    this->fileToWriteToFd = -1;

    memset(&serverAddress, 0, sizeof(serverAddress)); // do we need this?? 
    this->serverAddress.sin_port = htons(serverPort);
    this->serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    this->serverAddress.sin_family = AF_INET;
    this->serverAddressLength = sizeof(this->serverAddress);

    Helpers::BindAddressToSocket(this->serverAddress, this->socketFd);

    this->timeoutLimitVal = Helpers::ParseTimeoutLimitAsTimeval(timeoutLimit);

    this->maxNumberOfResendsAllowed = maxNumberOfResendsAllowed;
    this->currentNumberOfResends = 0;

    memset(&this->originalClientAddress, 0, sizeof(this->originalClientAddress));
    this->originalClientAddressLength = 0;

    memset(&this->currentClientAddress, 0, sizeof(this->currentClientAddress));
    this->currentClientAddressLength = 0;

    memset(this->dataBuffer, 0, maxDataBufferSize);

    this->numberOfBlocksRecieved = -1;
}

void Session::InitializeSocket()
{
    int sockedFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockedFd < 0)
    {
        cout << "Error: socket() failed" << endl;
        exit(1);
    }
    this->socketFd = sockedFd;
}

void Session::EndClientConnection()
{
    memset(&this->originalClientAddress, 0, sizeof(this->originalClientAddress));
    this->originalClientAddressLength = 0;

    memset(&currentClientAddress, 0, sizeof(currentClientAddress));
    currentClientAddressLength = 0;

    memset(dataBuffer, 0, maxDataBufferSize);

    this->currentNumberOfResends = 0;

    if (this->fileToWriteToFd >= 0)
    {
        int closeReturnValue = close(this->fileToWriteToFd);
        if (closeReturnValue < 0)
            Helpers::ExitProgramWithPERROR("close() failed");

        this->fileToWriteToFd = -1;
        this->fileToWriteToName = "";
    }

    this->numberOfBlocksRecieved = -1;
}

void Session::SendAckPacket()
{
    struct AckPacket ackPacket;
    ackPacket.blockNumber = htons(this->numberOfBlocksRecieved);
    int sendtoReturnValue = sendto(this->socketFd, &ackPacket, sizeof(ackPacket), 0, (struct sockaddr *)&this->originalClientAddress, this->originalClientAddressLength);
    if (sendtoReturnValue < 0)
        Helpers::ExitProgramWithPERROR("sendto() failed");
}

void Session::SendErrorPacket(short errorCode, string errorMessage)
{
    struct ErrorPacket errorPacket;
    errorPacket.errorCode = htons(errorCode);
    errorPacket.errorMessage = errorMessage;
    int sendtoReturnValue = sendto(this->socketFd, &errorPacket, sizeof(errorPacket), 0, (struct sockaddr *)&this->currentClientAddress, this->currentClientAddressLength);
    if (sendtoReturnValue < 0)
        Helpers::ExitProgramWithPERROR("sendto() failed");
}

int Session::RecievePacketFromClient()
{
    int recvfromReturnValue = recvfrom(this->socketFd, this->dataBuffer, maxDataBufferSize, 0, (struct sockaddr *)&(this->currentClientAddress), &(this->currentClientAddressLength));
    if (recvfromReturnValue < 0)
        Helpers::ExitProgramWithPERROR("recvfrom() failed");

    if(this->dataBuffer[0] == '1') // WRQ packet
    {
        if(this->numberOfBlocksRecieved != -1) // we are already in a session with another client
        {
            this->SendErrorPacket(4, "Unexpected packet");
            return GET_NEXT_PACKET;
        }

        else
        {
            this->HandleWrqPacket();
            return GET_NEXT_PACKET;
        }
            
    }

    else if(this->dataBuffer[0] == '3') // DATA packet
    {
        if(this->currentClientAddress.sin_addr.s_addr != this->originalClientAddress.sin_addr.s_addr) // we are in a session with another client
        {
            this->SendErrorPacket(4, "Unexpected packet");
            return GET_NEXT_PACKET;
        }

        else
            return this->HandleDataPacket();
    }
}

void Session::HandleWrqPacket()
{
    struct WrqPacket wrqPacket = Helpers::ParseBufferAsWrqPacket(this->dataBuffer);
    this->fileToWriteToName = wrqPacket.fileName;
    this->fileToWriteToFd = open(wrqPacket.fileName.c_str(), O_WRONLY | O_CREAT, 0666);
    if (this->fileToWriteToFd < 0)
        Helpers::ExitProgramWithPERROR("open() failed");

    this->numberOfBlocksRecieved = 0;
    this->currentNumberOfResends = 0;
    this->SendAckPacket();
}

int Session::HandleDataPacket()
{
    struct DataPacket dataPacket = Helpers::ParseBufferAsDataPacket(this->dataBuffer);
    if(ntohs(dataPacket.blockNumber) != this->numberOfBlocksRecieved + 1)
    {
        this->SendErrorPacket(4, "Bad block number");
        this->EndClientConnection();
        return END_CONNECTION;
    }
        return;
    int writeReturnValue = write(this->fileToWriteToFd, dataPacket.data, strlen(dataPacket.data));
    if (writeReturnValue < 0)
        Helpers::ExitProgramWithPERROR("write() failed");

    this->numberOfBlocksRecieved++;
    this->currentNumberOfResends = 0;
    this->SendAckPacket();
}