#ifndef CURRENT_DIR_HPP
#define CURRENT_DIR_HPP

#include <string>
#include <lua.hpp>

/**
 * Get the current working directory.
 * @return A string containing the current directory path if successful, or an error message if any.
 */
std::string currentDir();

/**
 * Lua binding for getting the current working directory.
 * @param L The Lua state.
 * @return Number of return values (1: current directory path or error message).
 * Lua usage: path_or_error = lua_currentDir()
 */
int lua_currentDir(lua_State* L);

#endif // CURRENT_DIR_HPP
