#ifndef CREATE_EXECUTABLE_HPP
#define CREATE_EXECUTABLE_HPP

#include <string>

/**
 * @brief Creates an executable file that includes a directory with a main.lua script.
 * @return true on success, false on any error (already logged to stderr).
 */
bool createExecutableWithDir(const std::string &dir, const std::string &output);

#endif // CREATE_EXECUTABLE_HPP
