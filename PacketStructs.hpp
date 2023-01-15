#ifndef PACKETSTRUCTS_HPP
#define PACKETSTRUCTS_HPP

#include <string>

#define MAX_DATA_SIZE 512
#define MAX_FILE_NAME_SIZE 257
#define MAX_TRANSMISSION_MODE_SIZE 257
#define MAX_ERROR_MESSAGE_SIZE 512

using namespace std;

struct WrqPacket
{
    const short opcode = 2;
    char fileName[MAX_FILE_NAME_SIZE];
    char transmissionMode[MAX_TRANSMISSION_MODE_SIZE];
}__attribute__((packed));

struct AckPacket
{
    const short opcode = 4;
    short blockNumber;
}__attribute__((packed));

struct DataPacket
{
    const short opcode = 3;
    short blockNumber;
    char data[MAX_DATA_SIZE];
}__attribute__((packed));

struct ErrorPacket
{
    const short opcode = 5;
    short errorCode;
    char errorMessage[MAX_ERROR_MESSAGE_SIZE];
}__attribute__((packed));

#endif