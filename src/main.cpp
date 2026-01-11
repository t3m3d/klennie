#include "ui.hpp"

#include <string>
#include <iostream>

int main(int argc, char* argv[])
{
    SortKey sortKey = SortKey::CPU;
    bool useColor = true;
    int refreshMillis = 1000;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--sort" && i + 1 < argc)
        {
            std::string val = argv[++i];
            if (val == "cpu")
                sortKey = SortKey::CPU;
            else if (val == "mem")
                sortKey = SortKey::RSS;
            else if (val == "pid")
                sortKey = SortKey::PID;
            else if (val == "name")
                sortKey = SortKey::NAME;
        }
        else if (arg == "--refresh" && i + 1 < argc)
        {
            refreshMillis = std::stoi(argv[++i]);
            if (refreshMillis < 100)
                refreshMillis = 100;
        }
        else if (arg == "--no-color")
        {
            useColor = false;
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout << "kprocview - Tiny Process Viewer\n"
            << "Usage: kprocview [options]\n"
            << "  --sort cpu|mem|pid|name   Initial sort key (default: cpu)\n"
            << "  --refresh ms              Refresh interval in ms (default: 1000)\n"
            << "  --no-color                Disable color output\n"
            << "  -h, --help                Show this help\n";
            return 0;
        }
    }

    UI ui(useColor, sortKey, refreshMillis);
    ui.run();
    return 0;
}
