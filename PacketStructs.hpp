#include <string>

using namespace std;

struct WrqPacket
{
    const short opcode = 2;
    string fileName;
    string transmissionMode;
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
    char data[512];
}__attribute__((packed));

struct ErrorPacket
{
    const short opcode = 5;
    short errorCode;
    string errorMessage; // what size?!±?! 
}__attribute__((packed));