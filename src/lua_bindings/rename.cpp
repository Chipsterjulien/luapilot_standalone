#include "lua_bindings/rename.hpp"
#include <cstdio>
#include <string>
#include <cerrno>
#include <cstring>

/**
 * @brief Function to rename a file or directory
 *
 * This function takes two strings representing the old path and the new path,
 * and renames the file or directory.
 *
 * @param old_path The current name of the file or directory
 * @param new_path The new name of the file or directory
 * @return A pair containing a boolean (true if the rename operation was successful, false otherwise)
 *         and a string with the error message if any.
 */
std::pair<bool, std::string> rename_file(const std::string& old_path, const std::string& new_path) {
    if (std::rename(old_path.c_str(), new_path.c_str()) != 0) {
        return {false, strerror(errno)};
    }
    return {true, ""};
}

/**
 * @brief Lua-accessible function to rename a file or directory
 *
 * This function is called from Lua and uses the rename_file function to rename a file or directory.
 * It expects to receive two strings as arguments.
 * If the arguments are not strings, a Lua error is raised.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (2 on success or failure: boolean result and error message)
 */
int lua_rename(lua_State* L) {
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        return luaL_error(L, "Expected two strings as arguments");
    }

    const char* old_path = lua_tostring(L, 1);
    const char* new_path = lua_tostring(L, 2);

    auto [success, error_message] = rename_file(old_path, new_path);
    lua_pushboolean(L, success);
    lua_pushstring(L, error_message.c_str());
    return 2;  // Two return values (boolean result and error message)
}
