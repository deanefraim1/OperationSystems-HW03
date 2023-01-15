#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include "PacketStructs.hpp"
#include "Session.hpp"
#include "Helpers.hpp"

using namespace std;

Session::Session(unsigned short serverPort, int timeoutLimit, int maxNumberOfResendsAllowed)
{
    this->InitializeSocket();
    this->serverAddress.InitializeAsServerAddress(serverPort);
    this->CleanDataBuffer();
    this->timeoutLimitVal = Helpers::ParseTimeoutLimitAsTimeval(timeoutLimit);
    this->maxNumberOfResendsAllowed = maxNumberOfResendsAllowed;
    this->currentNumberOfResends = 0;
    this->numberOfBlocksRecieved = EMPTY;
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
    this->originalClientAddress.CleanAddress();
    this->currentPacketClientAddress.CleanAddress();

    memset(packetDataBuffer, 0, MAX_BUFFER_SIZE);
    if (this->originalClientFileToWriteTo.fd != NOT_EXCIST)
        this->originalClientFileToWriteTo.CloseFile();
    this->currentNumberOfResends = 0;
    this->numberOfBlocksRecieved = EMPTY;
}

void Session::CleanDataBuffer()
{
    memset(this->packetDataBuffer, 0, MAX_BUFFER_SIZE);
}

void Session::SendAckPacket()
{
    struct AckPacket ackPacket;
    ackPacket.blockNumber = htons(this->numberOfBlocksRecieved);
    int sendtoReturnValue = sendto(this->socketFd, &ackPacket, sizeof(ackPacket), 0, (struct sockaddr *)&this->originalClientAddress, this->originalClientAddress.addressLength);
    if (sendtoReturnValue < 0)
        Helpers::ExitProgramWithPERROR("sendto() failed");
}

void Session::SendErrorPacketToCurrentClient(short errorCode, string errorMessage)
{
    struct ErrorPacket errorPacket;
    errorPacket.errorCode = htons(errorCode);
    memccpy(errorPacket.errorMessage, errorMessage.c_str(), '\0', errorMessage.length());
    int sendtoReturnValue = sendto(this->socketFd, &errorPacket, sizeof(errorPacket), 0, (struct sockaddr *)&this->currentPacketClientAddress, this->currentPacketClientAddress.addressLength);
    if (sendtoReturnValue < 0)
        Helpers::ExitProgramWithPERROR("sendto() failed");
}

void Session::SendErrorPacketToOriginalClient(short errorCode, string errorMessage)
{
    struct ErrorPacket errorPacket;
    errorPacket.errorCode = htons(errorCode);
    memccpy(errorPacket.errorMessage, errorMessage.c_str(), '\0', errorMessage.length());
    int sendtoReturnValue = sendto(this->socketFd, &errorPacket, sizeof(errorPacket), 0, (struct sockaddr *)&this->originalClientAddress, this->originalClientAddress.addressLength);
    if (sendtoReturnValue < 0)
        Helpers::ExitProgramWithPERROR("sendto() failed");
}

int Session::RecievePacketFromClient()
{
    int recvfromReturnValue;
    if (this->originalClientAddress.addressLength == 0) // we are not in a session with a client
        recvfromReturnValue = recvfrom(this->socketFd, this->packetDataBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&(this->originalClientAddress), &(this->originalClientAddress.addressLength));
    else // we are in a session with a client
        recvfromReturnValue = recvfrom(this->socketFd, this->packetDataBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&(this->currentPacketClientAddress), &(this->currentPacketClientAddress.addressLength));
    if (recvfromReturnValue < 0)
        Helpers::ExitProgramWithPERROR("recvfrom() failed");
    char packetOpcode = this->packetDataBuffer[0];
    if(packetOpcode == '2') // WRQ packet
    {
        if(this->originalClientAddress.addressLength != 0) // we are already in a session with another client
        {
            this->SendErrorPacketToCurrentClient(4, "Unexpected packet");
            return END_CONNECTION_FAILURE;
        }
        else
        {
            this->HandleWrqPacket();
            return GET_NEXT_PACKET;
        }
    }

    else if(packetOpcode == '3') // DATA packet
    {
        if(this->currentPacketClientAddress.address.sin_addr.s_addr != this->originalClientAddress.address.sin_addr.s_addr) // we are in a session with another client
        {
            this->SendErrorPacketToCurrentClient(4, "Unexpected packet");
            return GET_NEXT_PACKET;
        }
        else
            return this->HandleDataPacket();
    }
    else
    {
        if(this->currentPacketClientAddress.address.sin_addr.s_addr != this->originalClientAddress.address.sin_addr.s_addr) // we are in a session with another client
        {
            this->SendErrorPacketToCurrentClient(4, "Unexpected packet");
            return GET_NEXT_PACKET;
        }
        else
        {
            this->SendErrorPacketToOriginalClient(4, "Unexpected packet");
            return END_CONNECTION_FAILURE;
        }
    }
}

int Session::HandleWrqPacket()
{
    struct WrqPacket wrqPacket = Helpers::ParseBufferAsWrqPacket(this->packetDataBuffer);
    if(this->originalClientAddress.address.sin_addr.s_addr != NOT_EXCIST) // we are already in a session with another client
    {
        this->SendErrorPacketToCurrentClient(4, "Unexpected packet");
        return END_CONNECTION_FAILURE;
    }
    else if(FileManager::isFileExcist(wrqPacket.fileName)) 
    {
        this->SendErrorPacketToCurrentClient(6, "File already exists");
        return END_CONNECTION_FAILURE;
    }    
    else
    {
        this->originalClientFileToWriteTo = FileManager(wrqPacket.fileName);
        this->numberOfBlocksRecieved = 0;
        this->currentNumberOfResends = 0;
        this->SendAckPacket();
        return GET_NEXT_PACKET;
    }
}

int Session::HandleDataPacket()
{
    struct DataPacket dataPacket = Helpers::ParseBufferAsDataPacket(this->packetDataBuffer);
    if(this->originalClientAddress.address.sin_addr.s_addr == NOT_EXCIST) // we are not in a session with a client
    {
        this->SendErrorPacketToCurrentClient(4, "Unexpected packet");
        return END_CONNECTION_FAILURE;
    }
    else if(ntohs(dataPacket.blockNumber) != this->numberOfBlocksRecieved + 1) // wrong block number
    {
        this->SendErrorPacketToCurrentClient(0, "Bad block number");
        return END_CONNECTION_FAILURE;
    }
    else
    {
        this->originalClientFileToWriteTo.WriteToFile(dataPacket.data, strlen(dataPacket.data));
        this->numberOfBlocksRecieved++;
        this->currentNumberOfResends = 0;
        this->SendAckPacket();
        if(strlen(dataPacket.data) < MAX_DATA_SIZE) // last packet
            return END_CONNECTION_SUCCESS;
        else
            return GET_NEXT_PACKET;
    }
}