#ifndef LINK_HPP
#define LINK_HPP

#include <lua.hpp>
#include <optional>
#include <string>

/**
 * @brief Creates a symbolic link.
 *
 * This function is accessible from Lua et permet de créer un lien symbolique.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_link(lua_State* L);

#endif // LINK_HPP
