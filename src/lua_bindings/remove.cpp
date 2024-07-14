#include "remove.hpp"
#include <cstdio>
#include <string>
#include <system_error>

/**
 * @brief Function to remove a file
 *
 * This function takes a string representing the path of the file,
 * and removes the file.
 *
 * @param path The path of the file to remove
 * @return An error message if any, or an empty string if successful.
 */
std::string remove_file(const std::string& path) {
    std::error_code ec;
    if (std::remove(path.c_str()) != 0) {
        ec = std::error_code(errno, std::generic_category());
        return ec.message();
    }
    return "";
}

/**
 * @brief Lua-accessible function to remove a file
 *
 * This function is called from Lua and uses the remove_file function to remove a file.
 * It expects to receive a string as an argument.
 * If the argument is not a string, a Lua error is raised.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1: error message or nil).
 */
int lua_remove_file(lua_State* L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    const char* path = lua_tostring(L, 1);
    std::string error_message = remove_file(path);

    if (error_message.empty()) {
        lua_pushnil(L); // No error
    } else {
        lua_pushstring(L, error_message.c_str());
    }
    return 1;  // One return value (error message or nil)
}
