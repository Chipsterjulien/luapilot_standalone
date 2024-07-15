#include "executable_path.hpp"
#include <unistd.h>
#include <limits.h>
#include <stdexcept>

/**
 * @brief Gets the path of the currently running executable.
 *
 * This function reads the symbolic link /proc/self/exe to determine the path of the currently running executable.
 *
 * @return The path of the currently running executable.
 * @throws std::runtime_error if the executable path cannot be read.
 */
std::string getExecutablePath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count == -1) {
        throw std::runtime_error("Cannot read executable path");
    }
    std::string executablePath(result, count);
    return executablePath;
}

/**
 * @brief Gets the directory of the currently running executable.
 *
 * This function uses getExecutablePath to determine the directory containing the executable.
 *
 * @return The directory containing the currently running executable.
 */
std::string getExecutableDirectory() {
    std::string executablePath = getExecutablePath();
    size_t lastSlashPos = executablePath.find_last_of('/');
    return executablePath.substr(0, lastSlashPos);
}