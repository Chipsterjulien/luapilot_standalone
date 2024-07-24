#include "rmdir.hpp"
#include <filesystem>
#include <system_error>
#include <cstring>
#include <lua.hpp>
#include <optional>

namespace fs = std::filesystem;

/**
 * Check write permissions for a path.
 * @param path The path to check permissions for.
 * @return True if write permissions are available, false otherwise.
 */
bool has_write_permission(const fs::path& path) {
    std::error_code ec;
    auto perms = fs::status(path, ec).permissions();
    if (ec) {
        return false;
    }
    return (perms & fs::perms::owner_write) != fs::perms::none;
}

/**
 * Check if a path is a directory and has write permissions.
 * @param path The path to check.
 * @return An optional string with an error message, or nullopt if no errors.
 */
std::optional<std::string> check_path(std::string_view path) {
    std::error_code ec;
    fs::path dir_path(path);

    if (!fs::is_directory(dir_path, ec)) {
        if (ec) {
            return "Error checking if path is directory: " + std::string(path) + " : " + strerror(ec.value());
        }
        return "Path is not a directory: " + std::string(path);
    }

    if (!has_write_permission(dir_path)) {
        return "Permission denied: " + std::string(path);
    }

    return std::nullopt;
}

/**
 * Remove a file or an empty directory.
 * @param path The file or directory path to remove.
 * @return An optional string with the error message if any, or nullopt if successful.
 */
std::optional<std::string> rmdir(std::string_view path) {
    auto error = check_path(path);
    if (error) {
        return error;
    }

    std::error_code ec;
    fs::path dir_path(path);

    if (!fs::remove(dir_path, ec)) {
        if (ec) {
            return "Error removing directory: " + std::string(path) + " : " + strerror(ec.value());
        }
    }
    return std::nullopt;
}

/**
 * Remove a directory and all its contents.
 * @param path The directory path to remove.
 * @return An optional string with the error message if any, or nullopt if successful.
 */
std::optional<std::string> rmdir_all(std::string_view path) {
    auto error = check_path(path);
    if (error) {
        return error;
    }

    std::error_code ec;
    fs::path dir_path(path);

    fs::remove_all(dir_path, ec);
    if (ec) {
        return "Error removing directory and its contents: " + std::string(path) + " : " + strerror(ec.value());
    }
    return std::nullopt;
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

    auto error_message = rmdir(path);

    if (!error_message) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, error_message->c_str());
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

    auto error_message = rmdir_all(path);

    if (!error_message) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, error_message->c_str());
    }
    return 1;
}
