#ifndef LINK_HPP
#define LINK_HPP

#include <lua.hpp>

/**
 * @brief Creates a symbolic link.
 *
 * This function is exposed to Lua and creates a symbolic link at the specified path.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_link(lua_State* L);

#endif // LINK_HPP
