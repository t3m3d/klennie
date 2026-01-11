#include "ui.hpp"

#include <ncurses.h>
#include <chrono>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <sstream>

UI::UI(bool useColor, SortKey initialSort, int refreshMillis)
: sortKey_(initialSort),
refreshMillis_(refreshMillis)
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    initColors(useColor);
}

UI::~UI()
{
    endwin();
}

void UI::initColors(bool useColor)
{
    theme_.useColor = useColor;
    if (!useColor || !has_colors())
        return;

    start_color();
    use_default_colors();

    init_pair(theme_.headerPair, COLOR_CYAN, -1);
    init_pair(theme_.cpuHighPair, COLOR_RED, -1);
    init_pair(theme_.cpuMedPair, COLOR_YELLOW, -1);
    init_pair(theme_.cpuLowPair, COLOR_GREEN, -1);
    init_pair(theme_.borderPair, COLOR_MAGENTA, -1);
    init_pair(theme_.summaryPair, COLOR_BLUE, -1);
    init_pair(theme_.rowAltPair, COLOR_WHITE, -1);
}

void UI::run()
{
    while (true)
    {
        int ch = getch();
        if (ch == 'q' || ch == 'Q')
            break;

        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        int maxVisibleRows = rows - 6; // space for summary/header/borders

        handleInput(ch, maxVisibleRows);

        system_.update();
        applySorting();
        draw();

        std::this_thread::sleep_for(std::chrono::milliseconds(refreshMillis_));
    }
}

void UI::handleInput(int ch, int maxVisibleRows)
{
    if (ch == ERR)
        return;

    const auto& procs = system_.processes();
    int total = static_cast<int>(procs.size());

    switch (ch)
    {
        case 'c':
        case 'C':
            sortKey_ = SortKey::CPU;
            scrollOffset_ = 0;
            break;
        case 'm':
        case 'M':
            sortKey_ = SortKey::RSS;
            scrollOffset_ = 0;
            break;
        case 'p':
        case 'P':
            sortKey_ = SortKey::PID;
            scrollOffset_ = 0;
            break;
        case 'n':
        case 'N':
            sortKey_ = SortKey::NAME;
            scrollOffset_ = 0;
            break;
        case KEY_UP:
            if (scrollOffset_ > 0)
                --scrollOffset_;
        break;
        case KEY_DOWN:
            if (scrollOffset_ + maxVisibleRows < total)
                ++scrollOffset_;
        break;
        case KEY_PPAGE: // Page up
            scrollOffset_ -= maxVisibleRows;
            if (scrollOffset_ < 0)
                scrollOffset_ = 0;
        break;
        case KEY_NPAGE: // Page down
            scrollOffset_ += maxVisibleRows;
            if (scrollOffset_ + maxVisibleRows > total)
                scrollOffset_ = std::max(0, total - maxVisibleRows);
        break;
        default:
            break;
    }
}

void UI::applySorting()
{
    auto& procs = const_cast<std::vector<Process>&>(system_.processes());

    switch (sortKey_)
    {
        case SortKey::CPU:
            std::sort(procs.begin(), procs.end(),
                      [](const Process& a, const Process& b) {
                          return a.cpuPercent > b.cpuPercent;
                      });
            break;
        case SortKey::RSS:
            std::sort(procs.begin(), procs.end(),
                      [](const Process& a, const Process& b) {
                          return a.rssBytes > b.rssBytes;
                      });
            break;
        case SortKey::PID:
            std::sort(procs.begin(), procs.end(),
                      [](const Process& a, const Process& b) {
                          return a.pid < b.pid;
                      });
            break;
        case SortKey::NAME:
            std::sort(procs.begin(), procs.end(),
                      [](const Process& a, const Process& b) {
                          return a.name < b.name;
                      });
            break;
    }
}

void UI::draw()
{
    erase();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int currentRow = 0;

    if (theme_.useColor)
        attron(COLOR_PAIR(theme_.borderPair));
    mvhline(currentRow, 0, ACS_HLINE, cols);
    if (theme_.useColor)
        attroff(COLOR_PAIR(theme_.borderPair));

    ++currentRow;

    drawSummary(currentRow, cols);
    drawHeader(currentRow, cols);
    drawProcesses(currentRow, rows, cols);

    if (theme_.useColor)
        attron(COLOR_PAIR(theme_.borderPair));
    mvhline(rows - 1, 0, ACS_HLINE, cols);
    if (theme_.useColor)
        attroff(COLOR_PAIR(theme_.borderPair));

    refresh();
}

void UI::drawSummary(int& row, int cols)
{
    const auto& s = system_.summary();

    if (theme_.useColor)
        attron(COLOR_PAIR(theme_.summaryPair));

    std::string uptimeStr = formatUptime(s.uptimeSeconds);
    std::ostringstream oss;
    oss << "kprocview - Tiny Process Viewer  |  "
    << "Load: " << std::fixed << std::setprecision(2)
    << s.load1 << " " << s.load5 << " " << s.load15
    << "  |  Mem: " << s.memAvailableMiB << "MiB free / "
    << s.memTotalMiB << "MiB  |  Uptime: " << uptimeStr;

    mvaddnstr(row, 1, oss.str().c_str(), cols - 2);

    if (theme_.useColor)
        attroff(COLOR_PAIR(theme_.summaryPair));

    ++row;
    ++row;
}

void UI::drawHeader(int& row, int cols)
{
    if (theme_.useColor)
        attron(COLOR_PAIR(theme_.headerPair) | A_BOLD);

    std::string sortLabel;
    switch (sortKey_)
    {
        case SortKey::CPU: sortLabel = "CPU"; break;
        case SortKey::RSS: sortLabel = "MEM"; break;
        case SortKey::PID: sortLabel = "PID"; break;
        case SortKey::NAME: sortLabel = "NAME"; break;
    }

    std::ostringstream oss;
    oss << "PID      CPU%      BAR              RSS(MiB)   NAME"
    << "        [Sort: " << sortLabel
    << " | c=CPU m=MEM p=PID n=NAME | arrows=scroll | q=quit]";

    mvaddnstr(row, 1, oss.str().c_str(), cols - 2);

    if (theme_.useColor)
        attroff(COLOR_PAIR(theme_.headerPair) | A_BOLD);

    ++row;
}

void UI::drawProcesses(int startRow, int rows, int cols)
{
    const auto& procs = system_.processes();
    int total = static_cast<int>(procs.size());

    int maxVisible = rows - startRow - 1;
    if (maxVisible < 0)
        return;

    int endIndex = std::min(scrollOffset_ + maxVisible, total);

    int line = startRow;
    for (int i = scrollOffset_; i < endIndex; ++i)
    {
        const auto& p = procs[i];

        double rssMiB = static_cast<double>(p.rssBytes) / (1024.0 * 1024.0);

        int barWidth = 14;
        std::string bar = cpuBar(p.cpuPercent, barWidth);

        if (theme_.useColor)
        {
            if (i % 2 == 1)
                attron(COLOR_PAIR(theme_.rowAltPair));

            int pair = theme_.cpuLowPair;
            if (p.cpuPercent > 50.0)
                pair = theme_.cpuHighPair;
            else if (p.cpuPercent > 20.0)
                pair = theme_.cpuMedPair;

            attron(COLOR_PAIR(pair));
        }

        mvprintw(line, 1, "%-8d %6.2f   %-*s %9.2f   %-.*s",
                 p.pid,
                 p.cpuPercent,
                 barWidth, bar.c_str(),
                 rssMiB,
                 cols - 40, p.name.c_str());

        if (theme_.useColor)
        {
            attroff(COLOR_PAIR(theme_.cpuHighPair));
            attroff(COLOR_PAIR(theme_.cpuMedPair));
            attroff(COLOR_PAIR(theme_.cpuLowPair));
            if (i % 2 == 1)
                attroff(COLOR_PAIR(theme_.rowAltPair));
        }

        ++line;
    }
}

std::string UI::cpuBar(double cpuPercent, int width) const
{
    if (cpuPercent < 0.0)
        cpuPercent = 0.0;
    if (cpuPercent > 100.0)
        cpuPercent = 100.0;

    int filled = static_cast<int>((cpuPercent / 100.0) * width);
    if (filled > width)
        filled = width;

    std::string bar;
    bar.reserve(width);
    for (int i = 0; i < width; ++i)
    {
        bar.push_back(i < filled ? '#' : '-');
    }
    return bar;
}

std::string UI::formatUptime(std::uint64_t seconds) const
{
    std::uint64_t days = seconds / 86400;
    seconds %= 86400;
    std::uint64_t hours = seconds / 3600;
    seconds %= 3600;
    std::uint64_t minutes = seconds / 60;

    std::ostringstream oss;
    if (days > 0)
        oss << days << "d ";
    oss << hours << "h " << minutes << "m";
    return oss.str();
}
