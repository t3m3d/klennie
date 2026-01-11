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
    updateSummary();
}

void System::update()
{
    processes_.clear();

    std::uint64_t currentTotalCpu = readTotalCpuJiffies();
    std::uint64_t totalCpuDelta = currentTotalCpu - prevTotalCpu_;
    if (totalCpuDelta == 0)
        totalCpuDelta = 1;

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

    prevTotalCpu_ = currentTotalCpu;
    prevProcCpu_ = std::move(currentProcCpu);

    processes_ = std::move(current);

    updateSummary();
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

    std::ifstream statFile(base + "/stat");
    if (!statFile)
        return false;

    std::string statLine;
    std::getline(statFile, statLine);

    std::istringstream iss(statLine);
    int readPid;
    std::string comm;
    char state;
    std::uint64_t utime, stime;
    long rssPages;

    iss >> readPid >> comm >> state;

    std::string skip;
    for (int i = 0; i < 10; ++i)
        iss >> skip;

    iss >> utime >> stime;

    for (int i = 0; i < 7; ++i)
        iss >> skip;

    iss >> rssPages;

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

void System::updateSummary()
{
    readLoadAvg();
    readMemInfo();
    readUptime();
}

void System::readLoadAvg()
{
    std::ifstream f("/proc/loadavg");
    if (!f)
        return;

    f >> summary_.load1 >> summary_.load5 >> summary_.load15;
}

void System::readMemInfo()
{
    std::ifstream f("/proc/meminfo");
    if (!f)
        return;

    std::string key;
    std::uint64_t value;
    std::string unit;

    std::uint64_t memTotalKB = 0;
    std::uint64_t memFreeKB = 0;
    std::uint64_t memAvailKB = 0;

    while (f >> key >> value >> unit)
    {
        if (key == "MemTotal:")
            memTotalKB = value;
        else if (key == "MemFree:")
            memFreeKB = value;
        else if (key == "MemAvailable:")
            memAvailKB = value;
    }

    summary_.memTotalMiB = memTotalKB / 1024;
    summary_.memFreeMiB = memFreeKB / 1024;
    summary_.memAvailableMiB = memAvailKB / 1024;
}

void System::readUptime()
{
    std::ifstream f("/proc/uptime");
    if (!f)
        return;

    double upSeconds = 0.0;
    f >> upSeconds;
    summary_.uptimeSeconds = static_cast<std::uint64_t>(upSeconds);
}
