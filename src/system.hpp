#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <string>
#include "process.hpp"

struct SystemSummary
{
    double load1 = 0.0;
    double load5 = 0.0;
    double load15 = 0.0;
    std::uint64_t memTotalMiB = 0;
    std::uint64_t memFreeMiB = 0;
    std::uint64_t memAvailableMiB = 0;
    std::uptimeSeconds = 0;

};

class System
{
public:
    System();

    void update();

    const std::vector<Process>& processes() const { return processes_; }

private:
    std::vector<Process> processes_;
    SystemSummary summary_;

    std::unordered_map<int, std::uint64_t> prevProcCpu_;
    std::uint64_t prevTotalCpu_ = 0;

    std::uint64_t readTotalCpuJiffies() const;
    bool readProcessStat(int pid, Process& proc, std::uint64_t& totalProcJiffies) const;
    std::uint64_t readPageSize() const;

    void updateSummary();
    void readLoadAvg();
    void readMemInfo();
    void readUptime();
};
