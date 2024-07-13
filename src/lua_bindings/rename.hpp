#ifndef RENAME_HPP
#define RENAME_HPP

#include <lua.hpp>

/**
 * @brief Declaration of the lua_rename function for Lua
 *
 * This function is called from Lua to rename a file or directory.
 * It takes two strings representing the old path and the new path.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1: error message or nil)
 */
int lua_rename(lua_State* L);

#endif // RENAME_HPP
