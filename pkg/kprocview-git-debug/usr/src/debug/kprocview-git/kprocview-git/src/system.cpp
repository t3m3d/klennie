#include "system.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <unistd.h> // sysconf

namespace fs = std::filesystem;

System::System()
{
    prevTotalCpu_ = readTotalCpuJiffies();
}

void System::update()
{
    processes_.clear();

    std::uint64_t currentTotalCpu = readTotalCpuJiffies();
    std::uint64_t totalCpuDelta = currentTotalCpu - prevTotalCpu_;
    if (totalCpuDelta == 0)
        totalCpuDelta = 1; // avoid div by zero

    std::vector<Process> current;
    std::unordered_map<int, std::uint64_t> currentProcCpu;

    for (const auto& entry : fs::directory_iterator("/proc"))
    {
        if (!entry.is_directory())
            continue;

        const std::string pidStr = entry.path().filename().string();
        if (!std::all_of(pidStr.begin(), pidStr.end(), ::isdigit))
            continue;

        int pid = std::stoi(pidStr);
        Process proc;
        std::uint64_t procJiffies = 0;

        if (!readProcessStat(pid, proc, procJiffies))
            continue;

        currentProcCpu[pid] = procJiffies;

        std::uint64_t prevJiffies = 0;
        auto it = prevProcCpu_.find(pid);
        if (it != prevProcCpu_.end())
            prevJiffies = it->second;

        std::uint64_t delta = procJiffies - prevJiffies;
        proc.cpuPercent = (100.0 * static_cast<double>(delta)) /
                          static_cast<double>(totalCpuDelta);

        current.push_back(proc);
    }

    // Update previous CPU snapshots
    prevTotalCpu_ = currentTotalCpu;
    prevProcCpu_ = std::move(currentProcCpu);

    // Sort by CPU descending
    std::sort(current.begin(), current.end(),
              [](const Process& a, const Process& b) {
                  return a.cpuPercent > b.cpuPercent;
              });

    processes_ = std::move(current);
}

std::uint64_t System::readTotalCpuJiffies() const
{
    std::ifstream stat("/proc/stat");
    if (!stat)
        return 0;

    std::string line;
    if (!std::getline(stat, line))
        return 0;

    std::istringstream iss(line);
    std::string cpuLabel;
    std::uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
    // cpu  user nice system idle iowait irq softirq steal ...
    if (!(iss >> cpuLabel >> user >> nice >> system >> idle
              >> iowait >> irq >> softirq >> steal))
    {
        return 0;
    }

    return user + nice + system + idle + iowait + irq + softirq + steal;
}

bool System::readProcessStat(int pid, Process& proc, std::uint64_t& totalProcJiffies) const
{
    std::string base = "/proc/" + std::to_string(pid);

    // /proc/[pid]/stat
    std::ifstream statFile(base + "/stat");
    if (!statFile)
        return false;

    std::string statLine;
    std::getline(statFile, statLine);

    // /proc/[pid]/stat has multiple fields, we care about:
    // 1 pid
    // 2 comm (in parentheses)
    // 14 utime
    // 15 stime
    // 24 rss (in pages)
    std::istringstream iss(statLine);
    int readPid;
    std::string comm;
    char state;
    std::uint64_t utime, stime;
    long rssPages;
    // We'll read fields up to rss
    // There are many fields; we can extract stepwise.

    // Format: pid (comm) state ppid pgrp session tty_nr tpgid flags minflt cminflt
    //         majflt cmajflt utime stime cutime cstime priority nice num_threads
    //         itrealvalue starttime vsize rss ...
    iss >> readPid >> comm >> state;

    // Skip fields 4–13
    std::string skip;
    for (int i = 0; i < 10; ++i)
        iss >> skip;

    iss >> utime >> stime;

    // Skip cutime, cstime, priority, nice, num_threads, itrealvalue, starttime, vsize
    for (int i = 0; i < 7; ++i)
        iss >> skip;

    iss >> rssPages;

    // Process name: comm includes parentheses, e.g. (bash)
    // Strip parentheses.
    if (comm.size() >= 2 && comm.front() == '(' && comm.back() == ')')
        proc.name = comm.substr(1, comm.size() - 2);
    else
        proc.name = comm;

    proc.pid = pid;

    std::uint64_t pageSize = readPageSize();
    proc.rssBytes = static_cast<std::uint64_t>(rssPages) * pageSize;

    totalProcJiffies = utime + stime;
    return true;
}

std::uint64_t System::readPageSize() const
{
    long v = sysconf(_SC_PAGESIZE);
    if (v <= 0)
        return 4096;
    return static_cast<std::uint64_t>(v);
}