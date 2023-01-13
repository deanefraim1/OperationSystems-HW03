#ifndef HELPERS_HPP
#define HELPERS_HPP

#include "Session.hpp"
#include "PacketStructs.hpp"

class Helpers
{
public:
    static int InitializeSocket();
    static void BindSocket(Session session);
    static void RecieveMassage(Session session);
    static struct WRQ ParseBufferAsWrqPacket(char buffer[516]);
    static struct ACK ParseBufferAsAckPacket(char buffer[516]);
    static struct Data ParseBufferAsDataPacket(char buffer[516]);
};

#endif