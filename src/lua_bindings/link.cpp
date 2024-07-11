#include "lua_bindings/link.hpp"
#include <unistd.h>
#include <string>
#include <cstring>    // Pour strlen et strerror
#include <lua.hpp>
#include <limits.h>   // Pour PATH_MAX
#include <stdlib.h>   // Pour realpath
#include <iostream>
#include <cerrno>     // Pour errno

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

    if (std::strlen(target) >= PATH_MAX || std::strlen(linkpath) >= PATH_MAX) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Path too long");
        return 2;
    }

    std::string resolved_target(PATH_MAX, '\0');
    std::string resolved_linkpath(PATH_MAX, '\0');

    // Resolve the absolute path of the target
    if (realpath(target, &resolved_target[0]) == nullptr) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, std::strerror(errno));
        return 2;
    }

    // Resize the string to the actual length of the resolved path
    resolved_target.resize(std::strlen(resolved_target.c_str()));

    // Resolve the absolute path of the link path
    if (realpath(linkpath, &resolved_linkpath[0]) == nullptr) {
        // If the link path does not exist, it's not an error for creating a symlink,
        // so we just use the provided linkpath as it is.
        resolved_linkpath = linkpath;
    } else {
        // Resize the string to the actual length of the resolved path
        resolved_linkpath.resize(std::strlen(resolved_linkpath.c_str()));
    }

    // Debugging information
    std::cerr << "Creating symlink from '" << resolved_target << "' to '" << resolved_linkpath << "'" << std::endl;

    if (symlink(resolved_target.c_str(), resolved_linkpath.c_str()) != 0) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, std::strerror(errno));
        return 2;
    }

    lua_pushboolean(L, 1);
    lua_pushnil(L);
    return 2;
}
