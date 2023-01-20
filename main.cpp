#include "Helpers.hpp"
#include "Session.hpp"

int main(int argc, char *argv[])
{
    if (argc != 4)
        Helpers::ExitProgramWithPERROR("Invalid arguments");
    unsigned short serverPort = atoi(argv[1]);
    int timeoutLimit = atoi(argv[2]);
    int maxNumberOfResends = atoi(argv[3]);

    Session session(serverPort, timeoutLimit, maxNumberOfResends);

    while(1)
        Helpers::ReceiveMassage(session);

    return 0;
}