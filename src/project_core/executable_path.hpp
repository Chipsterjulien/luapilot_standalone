#ifndef EXECUTABLE_PATH_HPP
#define EXECUTABLE_PATH_HPP

#include <string>
#include <stdexcept>

/**
 * @brief Gets the path of the currently running executable.
 *
 * This function reads the symbolic link /proc/self/exe to determine the path of the currently running executable.
 *
 * @return The path of the currently running executable.
 * @throws std::runtime_error if the executable path cannot be read.
 */
std::string getExecutablePath();

/**
 * @brief Gets the directory of the currently running executable.
 *
 * This function uses getExecutablePath to determine the directory containing the executable.
 *
 * @return The directory containing the currently running executable.
 */
std::string getExecutableDirectory();

#endif // EXECUTABLE_PATH_HPP