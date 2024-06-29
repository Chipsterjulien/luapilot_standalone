#ifndef SETATTR_HPP
#define SETATTR_HPP

#include <lua.hpp>

/**
 * @brief Changes the attributes of a file or directory.
 *
 * This function is exposed to Lua and changes the attributes (mode, owner, group) of a specified file or directory.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_setattr(lua_State* L);

#endif // SETATTR_HPP
