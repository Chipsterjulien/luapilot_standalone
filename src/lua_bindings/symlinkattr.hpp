#ifndef SYMLINKATTR_HPP
#define SYMLINKATTR_HPP

#include <lua.hpp>

/**
 * @brief Changes the attributes of a symbolic link.
 *
 * This function is exposed to Lua and changes the attributes (owner, group) of a specified symbolic link.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_symlinkattr(lua_State* L);

#endif // SYMLINKATTR_HPP
