#ifndef DISK_INFO_H
#define DISK_INFO_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

/*
This is a basic class that represents a process. It includes important attributes, as indicated in the following list
*/
class DiskInfo
{

public:
    DiskInfo(const std::string &device, const std::string &capacity)
        : device(device), capacity(capacity) {}

    std::string getDevice() const { return device; }
    std::string getCapacity() const { return capacity; }

private:
    std::string device;
    std::string capacity;
};
#endif