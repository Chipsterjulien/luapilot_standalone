#ifndef SYMLINKATTR_HPP
#define SYMLINKATTR_HPP

#include <lua.hpp>

/**
 * @brief Lua binding: changes the owner and group of a symbolic link itself
 *        (does not follow the link, unlike a regular chown).
 *
 * Lua usage: ok, err = luapilot.symlinkattr(path, owner_uid, group_gid)
 *   - ok = true  on success, err = nil
 *   - ok = nil   on failure, err = error message
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack (2).
 */
int lua_symlinkattr(lua_State *L);

#endif // SYMLINKATTR_HPP