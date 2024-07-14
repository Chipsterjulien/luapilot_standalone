#ifndef CURRENT_DIR_HPP
#define CURRENT_DIR_HPP

#include <string>
#include <tuple>
#include <lua.hpp>

/**
 * Get the current working directory.
 * @return A tuple containing the current directory path and an error message if any.
 */
std::tuple<std::string, std::string> currentDir();

/**
 * Lua binding for getting the current working directory.
 * @param L The Lua state.
 * @return Number of return values (2: current directory path and error message).
 * Lua usage: currentDir, err = lua_currentDir()
 */
int lua_currentDir(lua_State* L);

#endif // CURRENT_DIR_HPP
