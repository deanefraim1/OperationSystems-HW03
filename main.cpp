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
#include "Session.hpp"

#define MAX_BUFFER_SIZE 512




int main(int argc, char *argv[])
{
    unsigned short serverPort = atoi(argv[1]);
    int timeout = atoi(argv[2]);
    int maxNumberOfResends = atoi(argv[3]);

    struct Session session;
    session.socketFd = Helpers::InitializeSocket();
    memset(&session.serverAddress, 0, sizeof(session.serverAddress));
    session.timeout = timeout;
    session.maxNumberOfResends = maxNumberOfResends;
    session.serverAddress.sin_port = htons(serverPort);
    session.serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    session.serverAddress.sin_family = AF_INET;
    
    Helpers::BindSocket(session);

    Helpers::RecieveMassage(session);
}