#ifndef CHDIR_HPP
#define CHDIR_HPP

#include <string>
#include <lua.hpp>

/**
 * Change the current working directory.
 * @param path The directory path to change to.
 * @return A string containing an error message if any, or an empty string if successful.
 */
std::string chdir(const std::string& path);

/**
 * Lua binding for changing the current working directory.
 * @param L The Lua state.
 * @return Number of return values (1: error message or nil).
 * Lua usage: error_message = lua_chdir(path)
 *   - path: The directory path to change to.
 */
int lua_chdir(lua_State* L);

#endif // CHDIR_HPP
