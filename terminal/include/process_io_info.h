#ifndef PROCESS_IO_INFO_H
#define PROCESS_IO_INFO_H

#include <curses.h>

class ProcessIoInfo
{
public:
    long long readBytes;
    long long writeBytes;
    std::string name;

    ProcessIoInfo() : readBytes(0), writeBytes(0) {}
};

#endif