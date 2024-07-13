#include "fileExists.hpp"
#include <filesystem>
#include <system_error>

/**
 * Check if a file exists and is a regular file.
 * @param path The file path to check.
 * @return A pair containing a boolean (true if the file exists and is a regular file, false otherwise)
 *         and a string with the error message if any.
 */
std::pair<bool, std::string> fileExists(const std::string& path) {
    std::error_code ec;
    if (std::filesystem::exists(path, ec)) {
        if (std::filesystem::is_regular_file(path, ec)) {
            return {true, ""}; // File exists and is a regular file
        } else {
            return {false, "Path exists but is not a regular file: " + path};
        }
    } else if (ec) {
        return {false, "Error checking file: " + ec.message()};
    }
    return {false, ""}; // File does not exist
}

/**
 * Lua binding for checking if a file exists.
 * @param L The Lua state.
 * @return Number of return values (2: fileFound boolean and error message or nil).
 * Lua usage: fileFound, err = lua_fileExists(path)
 *   - path: The file path to check.
 */
int lua_fileExists(lua_State* L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check if the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string");
    }

    // Get the file path from the Lua arguments
    const char* path = luaL_checkstring(L, 1);

    // Check if the file exists
    auto [fileFound, error_message] = fileExists(path);

    // Push the result onto the Lua stack
    lua_pushboolean(L, fileFound);
    if (error_message.empty()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, error_message.c_str());
    }

    return 2; // Two return values (fileFound boolean and error message or nil)
}
