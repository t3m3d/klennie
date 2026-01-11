#pragma once

#include <string>
#include <cstdint>

struct Process
{
    int pid = 0;
    std::string name;
    double cpuPercent = 0.0;
    std::uint64_t rssBytes = 0;
};