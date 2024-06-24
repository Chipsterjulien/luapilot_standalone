#ifndef FILE_EXISTS_HPP
#define FILE_EXISTS_HPP

#include <string>
#include <lua.hpp>

// Struct to hold the result of fileExists function
struct FileExistsResult {
    bool exists;
    std::string error_message;
};

/**
 * Check if a file exists and is a regular file.
 * @param path The file path to check.
 * @return A FileExistsResult struct containing the existence status and an error message if any.
 */
FileExistsResult fileExists(const std::string& path);

/**
 * Lua binding for checking if a file exists.
 * @param L The Lua state.
 * @return Number of return values (2: exists boolean and error message).
 * Lua usage: exists, error_message = lua_fileExists(path)
 *   - path: The file path to check.
 */
int lua_fileExists(lua_State* L);

#endif // FILE_EXISTS_HPP
