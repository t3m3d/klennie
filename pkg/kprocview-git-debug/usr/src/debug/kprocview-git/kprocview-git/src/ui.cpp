#include "ui.hpp"

#include <ncurses.h>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>

UI::UI()
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE); // non-blocking getch
    keypad(stdscr, TRUE);
}

UI::~UI()
{
    endwin();
}

void UI::run()
{
    while (true)
    {
        int ch = getch();
        if (ch == 'q' || ch == 'Q')
            break;

        system_.update();
        draw();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void UI::draw()
{
    erase();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    mvprintw(0, 0, "kprocview - Tiny Process Viewer (q to quit)");

    mvprintw(2, 0, "PID      CPU%%    RSS (MiB)   NAME");

    const auto& procs = system_.processes();
    int line = 3;

    for (const auto& p : procs)
    {
        if (line >= rows)
            break;

        double rssMiB = static_cast<double>(p.rssBytes) / (1024.0 * 1024.0);

        mvprintw(line, 0, "%-8d %6.2f   %8.2f   %.40s",
                 p.pid,
                 p.cpuPercent,
                 rssMiB,
                 p.name.c_str());

        ++line;
    }

    refresh();
}