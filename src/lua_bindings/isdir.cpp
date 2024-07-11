#include "lua_bindings/isdir.hpp"
#include <sys/stat.h>
#include <string>

/**
 * @brief Function to check if a path is a directory
 *
 * This function takes a string representing the path and checks if it is a directory.
 *
 * @param path The path to check
 * @return True if the path is a directory, false otherwise
 */
bool is_directory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false; // Path does not exist or error
    }
    return (info.st_mode & S_IFDIR) != 0;
}

/**
 * @brief Lua-accessible function to check if a path is a directory
 *
 * This function is called from Lua and uses the is_directory function to check if a given path is a directory.
 * It expects to receive a string as an argument.
 * If the argument is not a string, a Lua error is raised.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1 on success, the boolean result)
 */
int lua_isDir(lua_State* L) {
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    std::string path = lua_tostring(L, 1);
    bool result = is_directory(path);
    lua_pushboolean(L, result);
    return 1;  // One return value (the boolean result)
}
