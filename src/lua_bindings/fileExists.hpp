#ifndef FILE_EXISTS_HPP
#define FILE_EXISTS_HPP

#include <string>
#include <lua.hpp>
#include <utility> // for std::pair

/**
 * Check if a file exists and is a regular file.
 * @param path The file path to check.
 * @return A pair containing a boolean (true if the file exists and is a regular file, false otherwise)
 *         and a string with the error message if any.
 */
std::pair<bool, std::string> fileExists(const std::string& path);

/**
 * Lua binding for checking if a file exists.
 * @param L The Lua state.
 * @return Number of return values (2: fileFound boolean and error message or nil).
 * Lua usage: fileFound, err = lua_fileExists(path)
 *   - path: The file path to check.
 */
int lua_fileExists(lua_State* L);

#endif // FILE_EXISTS_HPP
