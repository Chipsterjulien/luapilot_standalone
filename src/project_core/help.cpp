#include "help.hpp"
#include "version.hpp"
#include <iostream>

void printHelp()
{
    std::cout
        << "Usage: luapilot [options] [script-dir]\n"
        << "\n"
        << "Options:\n"
        << "  -h, --help            Show this help and exit.\n"
        << "  -V, --version         Print version (luapilot "
        << LUAPILOT_VERSION_STRING
        << ") and exit.\n"
        << "  -c, --create-exe <dir> <output>\n"
        << "                        Create a self-contained executable by embedding\n"
        << "                        a directory (containing main.lua) into a copy\n"
        << "                        of luapilot.\n"
        << "\n"
        << "Execution modes:\n"
        << "  luapilot <directory>  Run <directory>/main.lua (folder mode).\n"
        << "  luapilot              When invoked as a binary produced by --create-exe,\n"
        << "                        run the embedded main.lua. Without an embedded\n"
        << "                        script, prints this help and exits with status 1.\n"
        << "\n"
        << "Examples:\n"
        << "  luapilot my_scripts\n"
        << "      Run my_scripts/main.lua.\n"
        << "\n"
        << "  luapilot --create-exe my_scripts myapp\n"
        << "      Bundle my_scripts/ into a standalone executable named 'myapp'.\n"
        << "\n"
        << "  ./myapp\n"
        << "      Run the embedded main.lua.\n"
        << "\n"
        << "See https://github.com/Chipsterjulien/luapilot_standalone for the full\n"
        << "documentation."
        << std::endl;
}
