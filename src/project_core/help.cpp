#include "help.hpp"
#include "version.hpp"
#include <iostream>

void printHelp()
{
    std::cout
        << "Usage: babet [options] [script-dir]\n"
        << "\n"
        << "Options:\n"
        << "  -h, --help            Show this help and exit.\n"
        << "  -V, --version         Print version (babet "
        << BABET_VERSION_STRING
        << ") and exit.\n"
        << "  -c, --create-exe <dir> <output>\n"
        << "                        Create a self-contained executable by embedding\n"
        << "                        a directory (containing main.lua) into a copy\n"
        << "                        of babet.\n"
        << "\n"
        << "Execution modes:\n"
        << "  babet <directory>  Run <directory>/main.lua (folder mode).\n"
        << "  babet              When invoked as a binary produced by --create-exe,\n"
        << "                        run the embedded main.lua. Without an embedded\n"
        << "                        script, prints this help and exits with status 1.\n"
        << "\n"
        << "Examples:\n"
        << "  babet my_scripts\n"
        << "      Run my_scripts/main.lua.\n"
        << "\n"
        << "  babet --create-exe my_scripts myapp\n"
        << "      Bundle my_scripts/ into a standalone executable named 'myapp'.\n"
        << "\n"
        << "  ./myapp\n"
        << "      Run the embedded main.lua.\n"
        << "\n"
        << "See https://github.com/Chipsterjulien/babet for the full\n"
        << "documentation."
        << std::endl;
}
