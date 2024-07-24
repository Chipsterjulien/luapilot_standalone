#ifndef FILEEXISTS_HPP
#define FILEEXISTS_HPP

#include <utility>
#include <string>
#include <filesystem>
#include <optional>
#include <string_view>
#include <lua.hpp>

/**
 * @brief Check if a file exists and is a regular file.
 *
 * @param path The file path to check.
 * @return std::optional<std::string> An optional containing an error message if any.
 */
std::optional<std::string> fileExists(std::string_view path);

/**
 * @brief Lua binding for checking if a file exists.
 *
 * @param L The Lua state.
 * @return int Number of return values (2: fileFound boolean and error message or nil).
 *
 * @note Lua usage: fileFound, err = lua_fileExists(path)
 *   - path: The file path to check.
 */
int lua_fileExists(lua_State* L);

#endif // FILEEXISTS_HPP
