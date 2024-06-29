#ifndef SETMODE_HPP
#define SETMODE_HPP

#include <lua.hpp>

/**
 * @brief Changes the permissions of a file or directory.
 *
 * This function is exposed to Lua and changes the permissions of a specified file or directory.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_setmode(lua_State* L);

#endif // SETMODE_HPP
