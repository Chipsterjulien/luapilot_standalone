#ifndef ISDIR_HPP
#define ISDIR_HPP

#include <lua.hpp>

/**
 * @brief Declaration of the lua_isDir function for Lua
 *
 * This function is called from Lua to check if a given path is a directory.
 * It takes a string representing the path and returns a boolean indicating whether it is a directory.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1 on success, the boolean result)
 */
int lua_isDir(lua_State* L);

#endif // ISDIR_HPP
