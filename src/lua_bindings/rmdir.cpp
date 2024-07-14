#include "rmdir.hpp"
#include <filesystem>
#include <system_error>

/**
 * Remove a file or an empty directory.
 * @param path The file or directory path to remove.
 * @return A string with the error message if any, or an empty string if successful.
 */
std::string rmdir(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        return "Directory does not exist: " + path;
    }
    if (!std::filesystem::is_directory(path)) {
        return "Path is not a directory: " + path;
    }
    std::error_code ec;
    if (!std::filesystem::remove(path, ec)) {
        return "Error removing directory: " + ec.message();
    }
    return "";
}

/**
 * Remove a directory and all its contents.
 * @param path The directory path to remove.
 * @return A string with the error message if any, or an empty string if successful.
 */
std::string rmdir_all(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        return "Directory does not exist: " + path;
    }
    if (!std::filesystem::is_directory(path)) {
        return "Path is not a directory: " + path;
    }
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
    if (ec) {
        return "Error removing directory: " + ec.message();
    }
    return "";
}

/**
 * Lua binding for removing a file or an empty directory.
 * @param L The Lua state.
 * @return Number of return values (1: error message or nil).
 * Lua usage: error_message = lua_rmdir(path)
 *   - path: The file or directory path to remove.
 */
int lua_rmdir(lua_State* L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check that the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    const char* path = luaL_checkstring(L, 1);

    std::string error_message = rmdir(path);

    if (error_message.empty()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, error_message.c_str());
    }
    return 1;
}

/**
 * Lua binding for removing a directory and all its contents.
 * @param L The Lua state.
 * @return Number of return values (1: error message or nil).
 * Lua usage: error_message = lua_rmdir_all(path)
 *   - path: The directory path to remove.
 */
int lua_rmdir_all(lua_State* L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check that the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    const char* path = luaL_checkstring(L, 1);

    std::string error_message = rmdir_all(path);

    if (error_message.empty()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, error_message.c_str());
    }
    return 1;
}
