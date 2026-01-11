#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>
#include "process.hpp"

class System
{
public:
    System();

    // Refresh process list and CPU usage
    void update();

    const std::vector<Process>& processes() const { return processes_; }

private:
    std::vector<Process> processes_;

    // For CPU usage calculation
    std::unordered_map<int, std::uint64_t> prevProcCpu_;
    std::uint64_t prevTotalCpu_ = 0;

    std::uint64_t readTotalCpuJiffies() const;
    bool readProcessStat(int pid, Process& proc, std::uint64_t& totalProcJiffies) const;
    std::uint64_t readPageSize() const;
};