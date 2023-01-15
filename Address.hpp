#ifndef ADDRESS_HPP
#define ADDRESS_HPP

#include <arpa/inet.h>

class Address
{
public:
    struct sockaddr_in address;
    socklen_t addressLength;

    Address();
    void InitializeAsServerAddress(unsigned short serverPort);
    void CleanAddress();
};

#endif