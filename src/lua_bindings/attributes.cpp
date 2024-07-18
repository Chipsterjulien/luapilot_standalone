#include "attributes.hpp"
#include <filesystem>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>
#include <lua.hpp>

namespace fs = std::filesystem;

/**
 * @brief Changes the owner and group of a file or directory.
 *
 * This function takes three arguments from the Lua stack:
 * 1. The path to the file or directory.
 * 2. The new owner (UID).
 * 3. The new group (GID).
 *
 * If it fails, it returns the error message to Lua.
 * If successful, it returns nil to Lua.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_setattr(lua_State* L) {
    // Check if there are three arguments passed
    int argc = lua_gettop(L);
    if (argc != 3) {
        return luaL_error(L, "Expected three arguments");
    }

    // Check that the arguments are of the correct types
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as first argument");
    }

    if (!lua_isinteger(L, 2)) {
        return luaL_error(L, "Expected an integer as second argument");
    }

    if (!lua_isinteger(L, 3)) {
        return luaL_error(L, "Expected an integer as third argument");
    }

    std::string path = luaL_checkstring(L, 1);
    uid_t owner = static_cast<uid_t>(luaL_checkinteger(L, 2));
    gid_t group = static_cast<gid_t>(luaL_checkinteger(L, 3));

    if (chown(path.c_str(), owner, group) != 0) {
        std::error_code ec(errno, std::generic_category());
        lua_pushstring(L, ec.message().c_str());
        return 1; // Return error message
    }

    lua_pushnil(L);
    return 1; // Return nil for success
}

/**
 * @brief Gets the attributes of a file or directory.
 *
 * This function takes one argument from the Lua stack:
 * 1. The path to the file or directory.
 *
 * If it fails, it returns the error message to Lua.
 * If successful, it returns a table with the attributes to Lua.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_getattr(lua_State* L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check that the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    std::string path = luaL_checkstring(L, 1);
    struct stat fileStat;

    if (stat(path.c_str(), &fileStat) != 0) {
        std::error_code ec(errno, std::generic_category());
        lua_pushnil(L);
        lua_pushstring(L, ec.message().c_str());
        return 2; // Return nil and error message
    }

    // Get permissions using filesystem library
    std::error_code ec;
    auto perms = fs::status(path, ec).permissions();
    if (ec) {
        lua_pushnil(L);
        lua_pushstring(L, ec.message().c_str());
        return 2; // Return nil and error message
    }

    // Create a table and push attributes
    lua_newtable(L);

    lua_pushstring(L, "mode");
    lua_pushinteger(L, static_cast<int>(perms) & 0777); // Mask to get only the permission bits
    lua_settable(L, -3);

    lua_pushstring(L, "owner");
    lua_pushinteger(L, fileStat.st_uid);
    lua_settable(L, -3);

    lua_pushstring(L, "group");
    lua_pushinteger(L, fileStat.st_gid);
    lua_settable(L, -3);

    return 1; // Return the table
}
