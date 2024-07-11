#ifndef COPY_HPP
#define COPY_HPP

#include <lua.hpp>

/**
 * @brief Declaration of the lua_copy_file function for Lua
 *
 * This function is called from Lua to copy a file from one path to another.
 * It takes two strings representing the source path and the destination path.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (2 on success or failure: boolean result and error message)
 */
int lua_copy_file(lua_State* L);

#endif // COPY_HPP
