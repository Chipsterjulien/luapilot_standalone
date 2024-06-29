#include "lua_bindings/setmode.hpp"
#include <sys/stat.h>
#include <errno.h>
#include <cstring>

/**
 * @brief Changes the permissions of a file or directory.
 *
 * This function takes two arguments from the Lua stack:
 * 1. The path to the file or directory.
 * 2. The new permissions (mode) in octal format.
 *
 * If successful, it returns true to Lua.
 * If it fails, it returns false and an error message.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_setmode(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    mode_t mode = static_cast<mode_t>(luaL_checkinteger(L, 2));

    if (chmod(path, mode) != 0) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, std::strerror(errno));
        return 2;
    }

    lua_pushboolean(L, 1);
    lua_pushnil(L);
    return 2;
}
