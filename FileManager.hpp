#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP

#include <string>

#define NOT_EXCIST -1

using namespace std;

class FileManager
{
public:
    int fd;
    string name;

    FileManager();
    FileManager(string fileName);
    void DeleteFile();
    void CloseFile();
    void WriteToFile(char* data, size_t size);
    static bool isFileExcist(string fileName);
};

#endif