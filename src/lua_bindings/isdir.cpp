#include "isdir.hpp"
#include <filesystem>
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
    return std::filesystem::is_directory(path);
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
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    std::string path = lua_tostring(L, 1);
    bool result = is_directory(path);
    lua_pushboolean(L, result);
    return 1;  // One return value (the boolean result)
}
