#pragma once

#include "system.hpp"

enum class SortKey
{
    CPU,
    RSS,
    PID,
    NAME
};

struct Theme
{
    bool useColor = true;
    int headerPair = 1;
    int cpuHighPair = 2;
    int cpuMedPair = 3;
    int cpuLowPair = 4;
    int borderPair = 5;
    int summaryPair = 6;
    int rowAltPair = 7;
};

class UI
{
public:
    UI(bool useColor, SortKey initialSort, int refreshMillis);
    ~UI();

    void run();

private:
    System system_;
    Theme theme_;
    SortKey sortKey_;
    int refreshMillis_;
    int scrollOffset_ = 0;

    void initColors(bool useColor);
    void draw();
    void drawSummary(int& row, int cols);
    void drawHeader(int& row, int cols);
    void drawProcesses(int startRow, int rows, int cols);
    void applySorting();
    void handleInput(int ch, int maxVisibleRows);
    std::string cpuBar(double cpuPercent, int width) const;
    std::string formatUptime(std::uint64_t seconds) const;
};
