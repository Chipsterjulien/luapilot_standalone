#include "mode.hpp"
#include <filesystem>
#include <system_error>
#include <lua.hpp>

namespace fs = std::filesystem;

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
    const char* mode_str = luaL_checkstring(L, 2);

    // Convert the mode string to an octal mode_t value
    char *endptr;
    mode_t mode = static_cast<mode_t>(strtol(mode_str, &endptr, 8));
    if (*endptr != '\0') {
        return luaL_error(L, "Invalid mode value");
    }

    std::error_code ec;
    fs::permissions(path, static_cast<fs::perms>(mode), ec);
    if (ec) {
        lua_pushstring(L, ec.message().c_str());
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

/**
 * @brief Gets the permissions of a file or directory.
 *
 * This function takes one argument from the Lua stack:
 * 1. The path to the file or directory.
 *
 * If it fails, it returns the error message to Lua.
 * If successful, it returns the permissions (mode) in octal format.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_getmode(lua_State* L) {
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

    std::error_code ec;
    auto perms = fs::status(path, ec).permissions();
    if (ec) {
        lua_pushnil(L);
        lua_pushstring(L, ec.message().c_str());
        return 2;
    }

    lua_pushinteger(L, static_cast<int>(perms) & 0777); // Mask to get only the permission bits
    lua_pushnil(L);
    return 2;
}
