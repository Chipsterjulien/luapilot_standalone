#ifndef REMOVE_HPP
#define REMOVE_HPP

#include <lua.hpp>

/**
 * @brief Declaration of the lua_remove_file function for Lua
 *
 * This function is called from Lua to remove a file.
 * It takes a string representing the path of the file to remove.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1: error message or nil)
 */
int lua_remove_file(lua_State* L);

#endif // REMOVE_HPP
