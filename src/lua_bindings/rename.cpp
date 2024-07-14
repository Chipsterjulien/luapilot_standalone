#include "rename.hpp"
#include <cstdio>
#include <string>
#include <system_error>

/**
 * @brief Function to rename a file or directory
 *
 * This function takes two strings representing the old path and the new path,
 * and renames the file or directory.
 *
 * @param old_path The current name of the file or directory
 * @param new_path The new name of the file or directory
 * @return A string with the error message if any, or an empty string if successful.
 */
std::string rename_file(const std::string& old_path, const std::string& new_path) {
    std::error_code ec;
    if (std::rename(old_path.c_str(), new_path.c_str()) != 0) {
        ec = std::error_code(errno, std::generic_category());
        return ec.message();
    }
    return "";
}

/**
 * @brief Lua-accessible function to rename a file or directory
 *
 * This function is called from Lua and uses the rename_file function to rename a file or directory.
 * It expects to receive two strings as arguments.
 * If the arguments are not strings, a Lua error is raised.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1: error message or nil).
 */
int lua_rename(lua_State* L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "Expected two arguments");
    }

    if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        return luaL_error(L, "Expected two strings as arguments");
    }

    const char* old_path = lua_tostring(L, 1);
    const char* new_path = lua_tostring(L, 2);

    std::string error_message = rename_file(old_path, new_path);

    if (error_message.empty()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, error_message.c_str());
    }
    return 1;  // One return value (error message or nil)
}
