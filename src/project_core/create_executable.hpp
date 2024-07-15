#ifndef CREATE_EXECUTABLE_HPP
#define CREATE_EXECUTABLE_HPP

#include <string>

/**
 * @brief Creates an executable file that includes a directory with a main.lua script.
 *
 * This function creates a ZIP file from the specified directory, merges it with the current executable,
 * and makes the resulting file executable.
 *
 * @param dir The directory containing the main.lua script and other files to include.
 * @param output The path to the output executable file.
 * @throws std::system_error if there is an error during the merge process.
 */
void createExecutableWithDir(const std::string &dir, const std::string &output);

#endif // CREATE_EXECUTABLE_HPP
