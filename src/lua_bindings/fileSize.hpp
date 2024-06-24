#ifndef FILE_SIZE_HPP
#define FILE_SIZE_HPP

#include <string>
#include <cstdint>
#include <lua.hpp>

// Struct to hold the result of fileSize function
struct FileSizeResult {
    bool success;
    std::uint64_t size;
    std::string error_message;
};

/**
 * Get the size of a file in bytes.
 * @param path The file path to check.
 * @return A FileSizeResult struct containing the size, success status, and an error message if any.
 */
FileSizeResult fileSize(const std::string& path);

/**
 * Lua binding for getting the size of a file.
 * @param L The Lua state.
 * @return Number of return values (2: size and error message).
 * Lua usage: size, error_message = lua_fileSize(path)
 *   - path: The file path to check.
 */
int lua_fileSize(lua_State* L);

#endif // FILE_SIZE_HPP
