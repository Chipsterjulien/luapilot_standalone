#include "help.hpp"
#include <iostream>

void printHelp() {
    std::cout << "Usage: luapilot [options] <arguments>\n"
              << "Options:\n"
              << "  -h, --help           Displays this help message.\n"
              << "  -c, --create-exe     Creates an executable by including a directory.\n"
              << "\n  Usage: -c <directory> <output>\n"
              << "  <directory>          Directory containing main.lua to execute.\n"
              << "  <output>             Name of the resulting executable.\n"
              << "\n\nIf no arguments are provided, the program\n"
              << "                       searches for and executes the main.lua embedded in the executable.\n"
              << "\nExamples:\n"
              << "  luapilot -c my_scripts luapilot_with_scripts\n"
              << "      Creates an executable named 'luapilot_with_scripts' including\n"
              << "      all files from the 'my_scripts' directory.\n"
              << "\n"
              << "  luapilot my_directory\n"
              << "      Executes the 'main.lua' file located in 'my_directory'.\n"
              << "\n"
              << "  luapilot\n"
              << "      If no arguments are provided, the program searches for and executes\n"
              << "      a 'main.lua' file embedded in the executable itself.\n"
              << std::endl;
}
