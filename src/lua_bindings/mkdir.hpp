#ifndef MKDIR_HPP
#define MKDIR_HPP

#include <lua.hpp>

/**
 * @brief Lua binding for creating a directory path.
 *
 * This function is called from Lua to create a directory. It takes a string representing the path
 * and an optional boolean to ignore if the directory already exists. It returns an error message
 * or nil if successful.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1: error message or nil)
 */
int lua_mkdir(lua_State* L);

#endif // MKDIR_HPP
