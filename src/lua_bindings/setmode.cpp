#include "setmode.hpp"
#include <sys/stat.h>
#include <system_error>

/**
 * @brief Changes the permissions of a file or directory.
 *
 * This function takes two arguments from the Lua stack:
 * 1. The path to the file or directory.
 * 2. The new permissions (mode) in octal format.
 *
 * If it fails, it returns the error message to Lua.
 * If successful, it returns nil to Lua.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_setmode(lua_State* L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "Expected two arguments");
    }

    // Check that the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as first argument");
    }

    if (!lua_isinteger(L, 2)) {
        return luaL_error(L, "Expected an integer as second argument");
    }

    const char* path = luaL_checkstring(L, 1);
    mode_t mode = static_cast<mode_t>(luaL_checkinteger(L, 2));

    if (chmod(path, mode) != 0) {
        std::error_code ec(errno, std::generic_category());
        lua_pushstring(L, ec.message().c_str());
        return 1;
    }

    lua_pushnil(L);
    return 1;
}
