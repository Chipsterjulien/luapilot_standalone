#ifndef SYMLINKATTR_HPP
#define SYMLINKATTR_HPP

#include <lua.hpp>

/**
 * @brief Changes the attributes of a symbolic link.
 *
 * This function takes three arguments from the Lua stack:
 * 1. The path to the symbolic link.
 * 2. The new owner (UID).
 * 3. The new group (GID).
 *
 * If it fails, it returns the error message to Lua.
 * If successful, it returns nil to Lua.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_symlinkattr(lua_State* L);

#endif // SYMLINKATTR_HPP
