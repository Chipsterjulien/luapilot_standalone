#include "attributes.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <system_error>

/**
 * @brief Changes the attributes of a file or directory.
 *
 * This function takes four arguments from the Lua stack:
 * 1. The path to the file or directory.
 * 2. The new permissions (mode) in octal format.
 * 3. The new owner (UID).
 * 4. The new group (GID).
 *
 * If it fails, it returns the error message to Lua.
 * If successful, it returns nil to Lua.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_setattr(lua_State* L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check that the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }


    const char* path = luaL_checkstring(L, 1);
    mode_t mode = static_cast<mode_t>(luaL_checkinteger(L, 2));
    uid_t owner = static_cast<uid_t>(luaL_checkinteger(L, 3));
    gid_t group = static_cast<gid_t>(luaL_checkinteger(L, 4));

    if (chmod(path, mode) != 0) {
        std::error_code ec(errno, std::generic_category());
        lua_pushstring(L, ec.message().c_str());
        return 1; // Return error message
    }

    if (chown(path, owner, group) != 0) {
        std::error_code ec(errno, std::generic_category());
        lua_pushstring(L, ec.message().c_str());
        return 1; // Return error message
    }

    lua_pushnil(L);
    return 1; // Return nil for success
}
