#include "FileManager.hpp"
#include "Helpers.hpp"

#include <fcntl.h>
#include <unistd.h>

FileManager::FileManager()
{
    this->name = "";
    this->fd = NOT_EXCIST;
}

FileManager::FileManager(string fileName)
{
    this->name = fileName;
    this->fd = open(fileName.c_str(), O_WRONLY | O_CREAT, 0666);
    if (this->fd == -1)
        Helpers::ExitProgramWithPERROR("open() failed");
}

void FileManager::DeleteFile()
{
    close(this->fd);
    remove(this->name.c_str());
    this->name = "";
    this->fd = NOT_EXCIST;
}

void FileManager::CloseFile()
{
    close(this->fd);
    this->name = "";
    this->fd = NOT_EXCIST;
}

void FileManager::WriteToFile(char* data, size_t size)
{
    int writeReturnValue = write(this->fd, data, size);
    if (writeReturnValue < 0)
        Helpers::ExitProgramWithPERROR("write() failed");
}

bool FileManager::isFileExcist(string fileName)
{
    int fd = open(fileName.c_str(), O_RDONLY, 0666);
    if (fd == -1) // file does not excist
        return false;
    close(fd);
    return true; 
}