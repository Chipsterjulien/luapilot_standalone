#ifndef ISFILE_HPP
#define ISFILE_HPP

#include <lua.hpp>

/**
 * @brief Declaration of the lua_isFile function for Lua
 *
 * This function is called from Lua to check if a given path is a file.
 * It takes a string representing the path and returns a boolean indicating whether it is a file.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1 on success, the boolean result)
 */
int lua_isFile(lua_State* L);

#endif // ISFILE_HPP
