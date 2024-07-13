#ifndef FILE_SIZE_HPP
#define FILE_SIZE_HPP

#include <string>
#include <cstdint>
#include <tuple>
#include <lua.hpp>

/**
 * Get the size of a file in bytes.
 * @param path The file path to check.
 * @return A tuple containing the size in bytes if successful, or an error message if any.
 */
std::tuple<std::uint64_t, std::string> fileSize(const std::string& path);

/**
 * Lua binding for getting the size of a file.
 * @param L The Lua state.
 * @return Number of return values (2: size and error message or nil).
 * Lua usage: size, error_message = lua_fileSize(path)
 *   - path: The file path to check.
 */
int lua_fileSize(lua_State* L);

#endif // FILE_SIZE_HPP
