#ifndef HELPERS_HPP
#define HELPERS_HPP

#include "Session.hpp"
#include "PacketStructs.hpp"
#include <sys/select.h>
#include "Address.hpp"

class Helpers
{
public:
    static void BindAddressToSocket(struct sockaddr_in address, int socketFd);
    static void ReceiveMassage(Session session);
    static short ParseOpcodeFromBuffer(char buffer[516]);
    static struct WrqPacket ParseBufferAsWrqPacket(char buffer[516]);
    static struct AckPacket ParseBufferAsAckPacket(char buffer[516]);
    static struct DataPacket ParseBufferAsDataPacket(char buffer[516]);
    static fd_set GetSocketFdSetFromOneSocketFd(int socketFd);
    static struct timeval ParseTimeoutLimitAsTimeval(int timeoutLimit);
    static void ExitProgramWithPERROR(string errorMessage);
    static void ExitProgramWithSTDERROR(string errorMessage);
    static bool AdressesAreEqual(Address address1, Address address2);
};

#endif