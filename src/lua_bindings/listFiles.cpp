#include "listFiles.hpp"
#include <iostream>
#include <system_error>
#include <optional>
#include <filesystem>

namespace fs = std::filesystem; // Alias pour std::filesystem

using optional_string = std::optional<std::string>;

/**
 * @brief Check if the directory path is valid.
 *
 * This function checks if the provided path exists and is a directory.
 *
 * @param path The directory path to check.
 * @return std::optional<std::string> An error message if the path is invalid, or std::nullopt if valid.
 */
optional_string validate_directory_path(const fs::path& path) {
    std::error_code ec;
    if (!fs::is_directory(path, ec)) {
        return ec ? ec.message() : "Path is not a directory";
    }
    return std::nullopt;
}

/**
 * Auxiliary function to list files.
 * @param L The Lua state.
 * @param basePath The base directory path.
 * @param path The current directory path.
 * @param index The index for the Lua table.
 * @param recursive Whether to list files recursively.
 * @return std::optional<std::string> An error message if an error occurs, or std::nullopt if successful.
 */
optional_string listFilesHelper(lua_State *L, const fs::path& basePath, const fs::path& path, int& index, bool recursive) {
    try {
        for (const auto &entry : fs::directory_iterator(path)) {
            if (fs::is_regular_file(entry)) {
                lua_pushnumber(L, index++);
                lua_pushstring(L, fs::relative(entry.path(), basePath).string().c_str());
                lua_settable(L, -3);
            }
            if (recursive && fs::is_directory(entry)) {
                if (auto error = listFilesHelper(L, basePath, entry.path(), index, recursive); error) {
                    return error;
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        return e.what();
    }
    return std::nullopt;
}

/**
 * Lua binding for listing files in a directory.
 * @param L The Lua state.
 * @return Number of return values (2: table of files and error message or nil).
 * Lua usage: files, err = lua_listFiles(path, recursive)
 *   - path: The directory path to list files from.
 *   - recursive (optional): Whether to list files recursively. Defaults to false.
 */
int lua_listFiles(lua_State *L) {
    // Ensure at least one argument is passed
    if (lua_gettop(L) < 1) {
        return luaL_error(L, "Expected at least one argument");
    }

    // Get the directory path from the first argument
    const char *path = luaL_checkstring(L, 1);
    bool recursive = lua_gettop(L) >= 2 ? lua_toboolean(L, 2) : false;

    // Validate the directory path
    fs::path canonical_path(path);
    if (auto error = validate_directory_path(canonical_path); error) {
        lua_pushnil(L);
        lua_pushstring(L, error->c_str());
        return 2; // Return nil and error message
    }

    lua_newtable(L); // Create a new table on the Lua stack

    int index = 1; // Initialize the index for the Lua table
    if (auto error = listFilesHelper(L, canonical_path, canonical_path, index, recursive); error) {
        lua_pushnil(L);
        lua_pushstring(L, error->c_str());
        return 2; // Return nil and the error message
    }

    lua_pushnil(L); // No error
    return 2; // Return the table of files and nil (no error)
}
