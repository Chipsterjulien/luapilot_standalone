#include "fileSize.hpp"
#include <filesystem>
#include <system_error>

/**
 * Get the size of a file in bytes.
 * @param path The file path to check.
 * @return A string containing the size in bytes if successful, or an error message if any.
 */
std::tuple<std::uint64_t, std::string> fileSize(const std::string& path) {
    std::error_code ec;
    if (std::filesystem::exists(path, ec) && std::filesystem::is_regular_file(path, ec)) {
        std::uint64_t size = std::filesystem::file_size(path, ec);
        if (!ec) {
            return {size, ""};
        } else {
            return {0, "Error getting file size: " + ec.message()};
        }
    } else {
        return {0, "File does not exist or is not a regular file: " + path};
    }
}

/**
 * Lua binding for getting the size of a file.
 * @param L The Lua state.
 * @return Number of return values (2: size and error message or nil).
 * Lua usage: size, error_message = lua_fileSize(path)
 *   - path: The file path to check.
 */
int lua_fileSize(lua_State* L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check if the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    // Get the file path from the Lua arguments
    const char* path = luaL_checkstring(L, 1);

    // Get the file size
    auto [size, error_message] = fileSize(path);

    // Check if getting the file size failed
    if (!error_message.empty()) {
        lua_pushnil(L);
        lua_pushstring(L, error_message.c_str());
        return 2;
    }

    // Push the file size onto the Lua stack
    lua_pushinteger(L, static_cast<lua_Integer>(size));
    lua_pushnil(L);  // No error message, so push nil
    return 2;
}
