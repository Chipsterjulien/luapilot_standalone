#include "link.hpp"
#include <unistd.h>
#include <string>
#include <system_error>
#include <limits.h>
#include <stdlib.h>
#include <cstring>  // For strlen
#include <iostream>

/**
 * @brief Creates a symbolic link.
 *
 * This function takes two arguments from the Lua stack:
 * 1. The target file or directory to link to.
 * 2. The path where the symbolic link will be created.
 *
 * If it fails, it returns the error message to Lua.
 * If successful, it returns nil to Lua.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_link(lua_State* L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "Expected two arguments");
    }

    const char* target = luaL_checkstring(L, 1);
    const char* linkpath = luaL_checkstring(L, 2);

    if (strlen(target) >= PATH_MAX || strlen(linkpath) >= PATH_MAX) {
        lua_pushstring(L, "Path too long");
        return 1;
    }

    std::string resolved_target(PATH_MAX, '\0');
    std::string resolved_linkpath(PATH_MAX, '\0');

    // Resolve the absolute path of the target
    if (realpath(target, &resolved_target[0]) == nullptr) {
        lua_pushstring(L, std::system_category().message(errno).c_str());
        return 1;
    }

    // Resize the string to the actual length of the resolved path
    resolved_target.resize(strlen(resolved_target.c_str()));

    // Resolve the absolute path of the link path
    if (realpath(linkpath, &resolved_linkpath[0]) == nullptr) {
        // If the link path does not exist, it's not an error for creating a symlink,
        // so we just use the provided linkpath as it is.
        resolved_linkpath = linkpath;
    } else {
        // Resize the string to the actual length of the resolved path
        resolved_linkpath.resize(strlen(resolved_linkpath.c_str()));
    }

    if (symlink(resolved_target.c_str(), resolved_linkpath.c_str()) != 0) {
        lua_pushstring(L, std::system_category().message(errno).c_str());
        return 1;
    }

    lua_pushnil(L);
    return 1;
}
