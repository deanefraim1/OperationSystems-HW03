#include <string>

using namespace std;

struct WRQ
{
    const char opcode[2] = "2";
    string filename;
    string transmissionMode;
}__attribute__((packed));

struct ACK
{
    const char opcode[2] = "4";
    char blockNumber[2];
}__attribute__((packed));

struct Data
{
    const char opcode[2] = "3";
    char blockNumber[2];
    char data[512];
}__attribute__((packed));