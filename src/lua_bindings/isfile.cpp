#include "lua_bindings/isfile.hpp"
#include <sys/stat.h>
#include <string>

/**
 * @brief Function to check if a path is a file
 *
 * This function takes a string representing the path and checks if it is a file.
 *
 * @param path The path to check
 * @return True if the path is a file, false otherwise
 */
bool is_file(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false; // Path does not exist or error
    }
    return (info.st_mode & S_IFREG) != 0;
}

/**
 * @brief Lua-accessible function to check if a path is a file
 *
 * This function is called from Lua and uses the is_file function to check if a given path is a file.
 * It expects to receive a string as an argument.
 * If the argument is not a string, a Lua error is raised.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1 on success, the boolean result)
 */
int lua_isFile(lua_State* L) {
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    std::string path = lua_tostring(L, 1);
    bool result = is_file(path);
    lua_pushboolean(L, result);
    return 1;  // One return value (the boolean result)
}
