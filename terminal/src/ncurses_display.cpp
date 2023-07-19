#include <chrono>
#include <curses.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <statgrab.h>

#include "format.h"
#include "ncurses_display.h"
#include "process_io_info.h"
#include "system.h"

using std::string;
using std::to_string;

// 50 bars uniformly displayed from 0 - 100 %
// 2% is one bar(|)
std::string NCursesDisplay::ProgressBar(float percent)
{
  std::string result{"[ "};
  int size{50};
  float bars{percent * size};

  for (int i{0}; i < size; ++i)
  {
    result += i <= bars ? '|' : ' ';
  }

  string display{to_string(percent * 100).substr(0, 4)};
  if (percent < 0.1 || percent == 1.0)
    display = " " + to_string(percent * 100).substr(0, 3);
  return result + " " + display + "/100% ]";
}

void NCursesDisplay::DisplaySystem(System &system, WINDOW *window)
{
  int row{0};
  mvwprintw(window, ++row, 2, ("OS: " + system.OperatingSystem()).c_str());
  mvwprintw(window, ++row, 2, "Disk:");
  wattron(window, COLOR_PAIR(3));
  float utilisationCpu = system.Cpu().Utilization();

  for (const auto &disk : system.getDiskInfo())
  {
    mvwprintw(window, ++row, 2, (disk.getDevice() + " - " + disk.getCapacity()).c_str());
  }
  wattroff(window, COLOR_PAIR(3));

  mvwprintw(window, ++row, 2, "Total CPU usage: ");
  wattron(window, COLOR_PAIR(1));
  mvwprintw(window, row, 27, "");
  wprintw(window, ProgressBar(utilisationCpu).c_str());
  wattroff(window, COLOR_PAIR(1));

  mvwprintw(window, ++row, 2, "Total Memory usage: ");
  wattron(window, COLOR_PAIR(1));
  mvwprintw(window, row, 26, " ");
  wprintw(window, ProgressBar(system.MemoryUtilization()).c_str());
  wattroff(window, COLOR_PAIR(1));

  mvwprintw(window, ++row, 2, "Free Memory: ");
  wattron(window, COLOR_PAIR(3));
  mvwprintw(window, row, 26, " ");
  wprintw(window, ProgressBar(system.MemoryAvailable()).c_str());
  wattroff(window, COLOR_PAIR(3));

  mvwprintw(window, ++row, 2,
            ("Total Processes: " + to_string(system.TotalProcesses())).c_str());
  mvwprintw(window, ++row, 2,
            ("Up Time: " + Format::ElapsedTime(system.UpTime())).c_str());
  wrefresh(window);
}

void NCursesDisplay::DisplayProcesses(std::vector<Process> &processes,
                                      WINDOW *window, int n)
{
  int row{0};
  int const pid_column{2};
  int const user_column{9};
  int const cpu_column{16};
  int const ram_column{26};
  int const time_column{35};
  int const command_column{46};
  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, pid_column, "PID");
  mvwprintw(window, row, user_column, "USER");
  mvwprintw(window, row, cpu_column, "CPU[%%]");
  mvwprintw(window, row, ram_column, "RAM[MB]");
  mvwprintw(window, row, time_column, "TIME+");
  mvwprintw(window, row, command_column, "COMMAND");
  wattroff(window, COLOR_PAIR(2));
  for (int i = 0; i < n; ++i)
  {
    mvwprintw(window, ++row, pid_column, to_string(processes[i].Pid()).c_str());
    mvwprintw(window, row, user_column, processes[i].User().c_str());
    float cpu = processes[i].getCpuUtilization() * 100;
    mvwprintw(window, row, cpu_column, to_string(cpu).substr(0, 4).c_str());
    mvwprintw(window, row, ram_column, processes[i].Ram().c_str());
    mvwprintw(window, row, time_column,
              Format::ElapsedTime(processes[i].UpTime()).c_str());
    mvwprintw(window, row, command_column,
              processes[i].Command().substr(0, window->_maxx - 46).c_str());
  }
}

void NCursesDisplay::DisplayDiskReadWrite(System &system, WINDOW *window)
{
  int row{0};
  int const pid_column{2};
  int const disk_read_column{9};
  int const disk_write_column{26};
  int const disk_process_name{46};
  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, pid_column, "PID");
  mvwprintw(window, row, disk_read_column, "DISK READ");
  mvwprintw(window, row, disk_write_column, "DISK WRITE");
  mvwprintw(window, row, disk_process_name, "Process");

  wattroff(window, COLOR_PAIR(2));

  std::unordered_map<int, ProcessIoInfo> ioInfo = system.getDiskIoInfo();

  // Print disk read rate and write rate per process
  for (const auto &entry : ioInfo)
  {
    int pid = entry.first;
    const ProcessIoInfo &processIO = entry.second;

    mvwprintw(window, ++row, pid_column, to_string(pid).c_str());
    mvwprintw(window, row, disk_read_column, to_string(processIO.readBytes).c_str());
    mvwprintw(window, row, disk_write_column, to_string(processIO.writeBytes).c_str());
    mvwprintw(window, row, disk_process_name, processIO.name.c_str());
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

void NCursesDisplay::Display(System &system, int n)
{
  initscr();
  noecho();
  cbreak();
  start_color();

  int x_max{getmaxx(stdscr)};
  WINDOW *system_window = newwin(14, x_max - 1, 0, 0);
  WINDOW *process_window =
      newwin(3 + n, x_max - 1, system_window->_maxy + 1, 0);

  WINDOW *disk_window =
      newwin(13, x_max - 1, process_window->_begy + process_window->_maxy + 1, 0);

  while (1)
  {
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    box(system_window, 0, 0);
    box(process_window, 0, 0);
    box(disk_window, 0, 0);
    DisplaySystem(system, system_window);
    DisplayProcesses(system.Processes(), process_window, n);
    DisplayDiskReadWrite(system, disk_window);
    wrefresh(system_window);
    wrefresh(process_window);
    wrefresh(disk_window);
    refresh();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  endwin();
}