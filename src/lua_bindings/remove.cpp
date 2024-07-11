#include "lua_bindings/remove.hpp"
#include <cstdio>
#include <string>
#include <cerrno>
#include <cstring>

/**
 * @brief Function to remove a file
 *
 * This function takes a string representing the path of the file,
 * and removes the file.
 *
 * @param path The path of the file to remove
 * @return A pair containing a boolean (true if the remove operation was successful, false otherwise)
 *         and a string with the error message if any.
 */
std::pair<bool, std::string> remove_file(const std::string& path) {
    if (std::remove(path.c_str()) != 0) {
        return {false, strerror(errno)};
    }
    return {true, ""};
}

/**
 * @brief Lua-accessible function to remove a file
 *
 * This function is called from Lua and uses the remove_file function to remove a file.
 * It expects to receive a string as an argument.
 * If the argument is not a string, a Lua error is raised.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (2 on success or failure: boolean result and error message)
 */
int lua_remove_file(lua_State* L) {
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    const char* path = lua_tostring(L, 1);
    auto [success, error_message] = remove_file(path);
    lua_pushboolean(L, success);
    lua_pushstring(L, error_message.c_str());
    return 2;  // Two return values (boolean result and error message)
}
