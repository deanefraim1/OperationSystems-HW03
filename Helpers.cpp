#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "Helpers.hpp"
#include "Session.hpp"
#include "PacketStructs.hpp"


using namespace std;

void Helpers::BindAddressToSocket(struct sockaddr_in address, socklen_t addressLength, int socketFd)
{
    int bindReturnValue = ::bind(socketFd, (struct sockaddr *)&address, addressLength);
    if (bindReturnValue < 0)
        Helpers::ExitProgramWithPERROR("bind() failed");
}

void Helpers::ReceiveMassage(Session session)
{
    while (true)
    {
        fd_set socketFdSet = Helpers::GetSocketFdSetFromOneSocketFd(session.socketFd);
        int selectReturnValue = select(session.socketFd + 1, &socketFdSet, NULL, NULL, &(session.timeoutLimitVal));
        if(selectReturnValue < 0) // select() failed
            Helpers::ExitProgramWithPERROR("select() failed");
        else if(selectReturnValue == 0) // timeout has reached
        {
            if(session.originalClient.addressLength == 0) // no client is connected
                return;
            else if(session.currentNumberOfResends >= session.maxNumberOfResendsAllowed) // no more resends allowed
            {
                if(session.originalClientFileToWriteTo.fd != NOT_EXIST) // if a file is open
                    session.originalClientFileToWriteTo.DeleteFile();       
                session.SendErrorPacketToOriginalClient(0, "Abandoning file transmission"); // send error packet to client
                session.EndClientConnection();
                return;
            }
            else
            {
                session.currentNumberOfResends++;
                session.SendAckPacket();
            }
        }
        else // packet is pending to be recieved
        {
            int recievePacketReturnValue = session.RecievePacketFromClient();
            if(recievePacketReturnValue == END_CONNECTION_FAILURE)
            {
                session.originalClientFileToWriteTo.DeleteFile();
                session.EndClientConnection();
                return;
            }
            else if(recievePacketReturnValue == END_CONNECTION_SUCCESS)
            {
                session.originalClientFileToWriteTo.CloseFile();
                session.EndClientConnection();
                return;
            }
            session.currentPacketClient.CleanAddress();
            session.CleanDataBuffer();
        } 
    }
}

short Helpers::ParseOpcodeFromBuffer(char buffer[516])
{
    short opcode;
    memcpy(&opcode, buffer, 2);
    return ntohs(opcode);
}

struct WrqPacket Helpers::ParseBufferAsWrqPacket(char buffer[516])
{
    struct WrqPacket wrqPacket;
    memccpy(wrqPacket.fileName, buffer + 2, '\0', MAX_FILE_NAME_SIZE); // we can assume that the file name is not longer than MAX_FILE_NAME_SIZE bytes
    return wrqPacket;
}

struct DataPacket Helpers::ParseBufferAsDataPacket(char buffer[516])
{
    struct DataPacket dataPacket;
    memcpy(&dataPacket.blockNumber, buffer + 2, 2);
    dataPacket.blockNumber = ntohs(dataPacket.blockNumber);
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
    memcpy(&ackPacket.blockNumber, buffer + 2, 2);  
    ackPacket.blockNumber = ntohs(ackPacket.blockNumber);
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
    perror(("TTFTP_ERROR: " + errormessage).c_str());
    exit(1);
}

void Helpers::ExitProgramWithSTDERROR(string errorMessage)
{
    cout << "TTFTP_ERROR: " << errorMessage << endl;
    exit(1);
}

bool Helpers::AdressesAreEqual(Address address1, Address address2)
{
    return (address1.address.sin_addr.s_addr == address2.address.sin_addr.s_addr) &&
            (address1.address.sin_port == address2.address.sin_port);
}