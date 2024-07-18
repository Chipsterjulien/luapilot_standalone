#ifndef ATTRIBUTES_HPP
#define ATTRIBUTES_HPP

#include <lua.hpp>

/**
 * @brief Changes the attributes of a file or directory.
 *
 * This function is exposed to Lua and changes the attributes (mode, owner, group) of a specified file or directory.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_getattr(lua_State* L);
int lua_setattr(lua_State* L);

#endif // ATTRIBUTES_HPP
