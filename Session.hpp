#ifndef SESSION_HPP
#define SESSION_HPP

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 512

struct Session
{
public:
    int socketFd;
    struct sockaddr_in serverAddress;
    socklen_t serverAddressLength;
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength;
    char buffer[MAX_BUFFER_SIZE];
    const int maxBufferSize = MAX_BUFFER_SIZE;
    int timeout;
    int maxNumberOfResends;
};

#endif