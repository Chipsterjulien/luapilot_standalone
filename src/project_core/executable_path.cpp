#include "executable_path.hpp"
#include <unistd.h>
#include <limits.h>

std::string getExecutablePath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count == -1) {
        throw std::runtime_error("Cannot read executable path");
    }
    std::string executablePath(result, count);
    return executablePath;
}

std::string getExecutableDirectory() {
    std::string executablePath = getExecutablePath();
    size_t lastSlashPos = executablePath.find_last_of('/');
    return executablePath.substr(0, lastSlashPos);
}
