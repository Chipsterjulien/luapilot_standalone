#include "symlinkattr.hpp"
#include <unistd.h>
#include <system_error>

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
int lua_symlinkattr(lua_State* L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 3) {
        return luaL_error(L, "Expected three arguments");
    }

    // Check that the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    // Check that the second argument is an integer
    if (!lua_isinteger(L, 2)) {
        return luaL_error(L, "Expected an integer as the second argument");
    }

    // Check that the third argument is an integer
    if (!lua_isinteger(L, 3)) {
        return luaL_error(L, "Expected an integer as the third argument");
    }

    const char* path = luaL_checkstring(L, 1);
    uid_t owner = static_cast<uid_t>(luaL_checkinteger(L, 2));
    gid_t group = static_cast<gid_t>(luaL_checkinteger(L, 3));

    if (lchown(path, owner, group) != 0) {
        std::error_code ec(errno, std::generic_category());
        lua_pushstring(L, ec.message().c_str());
        return 1; // Return error message
    }

    lua_pushnil(L);
    return 1; // Return nil for success
}
