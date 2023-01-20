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
    this->serverAddress.InitializeAsServerAddress(this->socketFd, serverPort);
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
    this->originalClient.CleanAddress();
    this->currentPacketClient.CleanAddress();

    this->CleanDataBuffer();
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
    int sendtoReturnValue = sendto(this->socketFd, &ackPacket, sizeof(ackPacket), 0, (struct sockaddr *)&this->originalClient, this->originalClient.addressLength);
    if (sendtoReturnValue < 0)
        Helpers::ExitProgramWithPERROR("sendto() failed");
}

void Session::SendErrorPacketToCurrentPacketClient(short errorCode, string errorMessage)
{
    struct ErrorPacket errorPacket;
    errorPacket.errorCode = htons(errorCode);
    memccpy(errorPacket.errorMessage, errorMessage.c_str(), '\0', errorMessage.length());
    int sendtoReturnValue = sendto(this->socketFd, &errorPacket, sizeof(errorPacket), 0, (struct sockaddr *)&this->currentPacketClient, this->currentPacketClient.addressLength);
    if (sendtoReturnValue < 0)
        Helpers::ExitProgramWithPERROR("sendto() failed");
}

void Session::SendErrorPacketToOriginalClient(short errorCode, string errorMessage)
{
    struct ErrorPacket errorPacket;
    errorPacket.errorCode = htons(errorCode);
    memccpy(errorPacket.errorMessage, errorMessage.c_str(), '\0', errorMessage.length());
    int sendtoReturnValue = sendto(this->socketFd, &errorPacket, sizeof(errorPacket), 0, (struct sockaddr *)&this->originalClient, this->originalClient.addressLength);
    if (sendtoReturnValue < 0)
        Helpers::ExitProgramWithPERROR("sendto() failed");
}

int Session::RecievePacketFromClient()
{
    int recvfromReturnValue;
    short packetOpcode;
    if (this->originalClient.addressLength == 0) // we are not in a session with a client
    {
        recvfromReturnValue = recvfrom(this->socketFd, this->packetDataBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&(this->originalClient.address), &(this->originalClient.addressLength));
        if (recvfromReturnValue < 0)
            Helpers::ExitProgramWithPERROR("recvfrom() failed");
        packetOpcode = Helpers::ParseOpcodeFromBuffer(this->packetDataBuffer);
        if(packetOpcode == 2) // WRQ packet
        {
            return this->HandleWrqPacket();
        }
        else
        {
            this->SendErrorPacketToCurrentPacketClient(7, "Unknown user");
            return GET_NEXT_PACKET;
        }
    }
    else // we are in a session with a client
    {
        recvfromReturnValue = recvfrom(this->socketFd, this->packetDataBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&(this->currentPacketClient.address), &(this->currentPacketClient.addressLength));
        if (recvfromReturnValue < 0)
            Helpers::ExitProgramWithPERROR("recvfrom() failed");
        packetOpcode = Helpers::ParseOpcodeFromBuffer(this->packetDataBuffer);
        if(!Helpers::AdressesAreEqual(this->currentPacketClient, this->originalClient)) // we got a packet from a different client
        {
            this->SendErrorPacketToCurrentPacketClient(4, "Unexpected packet"); // TODO - shuld we ++ the number of resends?
            return GET_NEXT_PACKET;
        }
        else
        {
            if(packetOpcode == 2) // WRQ packet
            {
                this->SendErrorPacketToOriginalClient(4, "Unexpected packet");
                return GET_NEXT_PACKET; //TODO - should we end connection with original client?!
            }
            else if(packetOpcode == 3) // DATA packet
            {
                return this->HandleDataPacket();
            }
            else
            {
                this->SendErrorPacketToOriginalClient(4, "Unexpected packet");
                return GET_NEXT_PACKET;//TODO - should we end connection with original client?!
            }
        }
    }
}

int Session::HandleWrqPacket()
{
    struct WrqPacket wrqPacket = Helpers::ParseBufferAsWrqPacket(this->packetDataBuffer);
    if(FileManager::isFileExcist(wrqPacket.fileName)) 
    {
        this->SendErrorPacketToOriginalClient(6, "File already exists");
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
    if(dataPacket.blockNumber != this->numberOfBlocksRecieved + 1) // wrong block number
    {
        this->SendErrorPacketToOriginalClient(0, "Bad block number");
        return END_CONNECTION_FAILURE; //TODO - should we end connection with original client?!
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