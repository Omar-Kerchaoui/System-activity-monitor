#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>
#include <unordered_map>

#include "process.h"
#include "processor.h"
#include "process_io_info.h"
#include "disk_info.h"

class System
{
public:
  Processor &Cpu();
  std::vector<Process> &Processes();
  float MemoryUtilization();
  float MemoryAvailable();
  long UpTime();
  int TotalProcesses();
  int RunningProcesses();
  std::string Kernel();
  std::string OperatingSystem();
  // disk
  std::vector<DiskInfo> getDiskInfo();
  std::unordered_map<int, ProcessIoInfo> getDiskIoInfo();

private:
  // Composition: System "has a" Processor called cpu
  Processor cpu_ = {};
  std::vector<Process> processes_ = {};
};

#endif