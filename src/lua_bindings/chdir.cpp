#include "chdir.hpp"
#include <filesystem>
#include <string>
#include <system_error>

/**
 * Change the current working directory.
 * @param path The directory path to change to.
 * @return A string containing an error message if any, or an empty string if successful.
 */
std::string chdir(const std::string& path) {
    std::error_code ec;
    std::filesystem::current_path(path, ec);
    if (ec) {
        return "Error changing directory: " + ec.message();
    }
    return "";
}

/**
 * Lua binding for changing the current working directory.
 * @param L The Lua state.
 * @return Number of return values (1: error message or nil).
 * Lua usage: error_message = lua_chdir(path)
 *   - path: The directory path to change to.
 */
int lua_chdir(lua_State* L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check if the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    const char* path = luaL_checkstring(L, 1);

    std::string error_message = chdir(path);

    if (error_message.empty()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, error_message.c_str());
    }
    return 1;
}
