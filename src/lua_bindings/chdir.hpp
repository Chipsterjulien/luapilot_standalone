#ifndef CHDIR_HPP
#define CHDIR_HPP

#include <string>
#include <optional>
#include <filesystem>
#include <lua.hpp>
#include "lua_utils.hpp"

/**
 * @brief Change the current working directory.
 *
 * @param path The directory path to change to.
 * @return std::optional<std::string> An optional string containing an error message if any, or an empty optional if successful.
 */
std::optional<std::string> chdir(const std::filesystem::path& path);

/**
 * @brief Lua binding for changing the current working directory.
 *
 * @param L The Lua state.
 * @return int Number of return values (1: error message or nil).
 * @note Lua usage: error_message = lua_chdir(path)
 *   - path: The directory path to change to.
 */
int lua_chdir(lua_State* L);

#endif // CHDIR_HPP
