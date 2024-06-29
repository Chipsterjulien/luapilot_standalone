#include "lua_bindings/symlinkattr.hpp"
#include <unistd.h>
#include <errno.h>
#include <cstring>

/**
 * @brief Changes the attributes of a symbolic link.
 *
 * This function takes three arguments from the Lua stack:
 * 1. The path to the symbolic link.
 * 2. The new owner (UID).
 * 3. The new group (GID).
 *
 * If successful, it returns true to Lua.
 * If it fails, it returns false and an error message.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_symlinkattr(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    uid_t owner = static_cast<uid_t>(luaL_checkinteger(L, 2));
    gid_t group = static_cast<gid_t>(luaL_checkinteger(L, 3));

    if (lchown(path, owner, group) != 0) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, std::strerror(errno));
        return 2;
    }

    lua_pushboolean(L, 1);
    lua_pushnil(L);
    return 2;
}
