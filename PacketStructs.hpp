#ifndef PACKETSTRUCTS_HPP
#define PACKETSTRUCTS_HPP

#include <string>
#include <arpa/inet.h>

#define MAX_DATA_SIZE 512
#define MAX_FILE_NAME_SIZE 508
#define TRANSMISSION_MODE_SIZE 6
#define MAX_ERROR_MESSAGE_SIZE 512

using namespace std;

struct WrqPacket
{
    const short opcode = ntohs(2);
    char fileName[MAX_FILE_NAME_SIZE];
    const char transmissionMode[TRANSMISSION_MODE_SIZE] = "octet";
}__attribute__((packed));

struct AckPacket
{
    const short opcode = htons(4);
    short blockNumber;
}__attribute__((packed));

struct DataPacket
{
    const short opcode = ntohs(3);
    short blockNumber;
    char data[MAX_DATA_SIZE];
}__attribute__((packed));

struct ErrorPacket
{
    const short opcode = htons(5);
    short errorCode;
    char errorMessage[MAX_ERROR_MESSAGE_SIZE];
}__attribute__((packed));

#endif