#include "lua_bindings/link.hpp"
#include <unistd.h>
#include <string>
#include <errno.h>
#include <cstring>
#include <lua.hpp>
#include <limits.h> // For PATH_MAX
#include <stdlib.h> // For realpath
#include <iostream>

/**
 * @brief Creates a symbolic link.
 *
 * This function takes two arguments from the Lua stack:
 * 1. The target file or directory to link to.
 * 2. The path where the symbolic link will be created.
 *
 * If successful, it returns true to Lua.
 * If it fails, it returns false and an error message.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_link(lua_State* L) {
    const char* target = luaL_checkstring(L, 1);
    const char* linkpath = luaL_checkstring(L, 2);

    char resolved_target[PATH_MAX];
    char resolved_linkpath[PATH_MAX];

    // Resolve the absolute path of the target
    if (realpath(target, resolved_target) == NULL) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, std::strerror(errno));
        return 2;
    }

    // Resolve the absolute path of the link path
    if (realpath(linkpath, resolved_linkpath) == NULL) {
        // If the link path does not exist, it's not an error for creating a symlink,
        // so we just use the provided linkpath as it is.
        strncpy(resolved_linkpath, linkpath, PATH_MAX);
    }

    // Debugging information
    std::cout << "Creating symlink from '" << resolved_target << "' to '" << resolved_linkpath << "'" << std::endl;

    if (symlink(resolved_target, resolved_linkpath) != 0) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, std::strerror(errno));
        return 2;
    }

    lua_pushboolean(L, 1);
    lua_pushnil(L);
    return 2;
}

extern "C" int luaopen_link(lua_State* L) {
    static const struct luaL_Reg linklib [] = {
        {"create_symlink", lua_link},
        {NULL, NULL}
    };
    luaL_newlib(L, linklib);
    return 1;
}
