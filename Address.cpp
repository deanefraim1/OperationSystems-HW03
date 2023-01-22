#include "Address.hpp"
#include "Helpers.hpp"
#include <cstring>

Address::Address()
{
    CleanAddress();
}

void Address::InitializeAsServerAddress(int socketFd, unsigned short serverPort)
{
    memset(&(this->address), 0, sizeof(this->address));
    address.sin_family = AF_INET;
    address.sin_port = htons(serverPort);
    address.sin_addr.s_addr = INADDR_ANY;
    addressLength = sizeof(address);
    Helpers::BindAddressToSocket(this->address, this->addressLength, socketFd);
}

void Address::CleanAddress()
{
    memset(&(this->address), 0, sizeof(this->address));
    this->addressLength = sizeof(this->address);
}