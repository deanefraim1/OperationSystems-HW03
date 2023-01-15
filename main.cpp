#include "Helpers.hpp"
#include "Session.hpp"

#define MAX_BUFFER_SIZE 512

int main(int argc, char *argv[])
{
    unsigned short serverPort = atoi(argv[1]);
    int timeoutLimit = atoi(argv[2]);
    int maxNumberOfResends = atoi(argv[3]);

    Session session(serverPort, timeoutLimit, maxNumberOfResends);

    while(1)
        Helpers::ReceiveMassage(session);
}