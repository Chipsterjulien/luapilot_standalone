#include "rmdir.hpp"
#include <filesystem>

/**
 * Remove a file or an empty directory.
 * @param path The file or directory path to remove.
 * @return A RemovePathResult struct containing the success status and an error message if any.
 */
RemovePathResult rmdir(const std::string& path) {
    RemovePathResult result = {true, ""};
    std::error_code ec;
    if (!std::filesystem::remove(path, ec)) {
        result.success = false;
        if (ec) {
            result.error_message = "Error removing file or directory: " + ec.message();
        } else {
            result.error_message = "File or directory does not exist: " + path;
        }
    }
    return result;
}

/**
 * Remove a directory and all its contents.
 * @param path The directory path to remove.
 * @return A RemovePathResult struct containing the success status and an error message if any.
 */
RemovePathResult rmdir_all(const std::string& path) {
    RemovePathResult result = {true, ""};
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
    if (ec) {
        result.success = false;
        result.error_message = "Error removing directory: " + ec.message();
    }
    return result;
}

/**
 * Lua binding for removing a file or an empty directory.
 * @param L The Lua state.
 * @return Number of return values (2: success boolean and error message).
 * Lua usage: success, error_message = lua_rmdir(path)
 *   - path: The file or directory path to remove.
 */
int lua_rmdir(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);

    RemovePathResult result = rmdir(path);

    lua_pushboolean(L, result.success);
    lua_pushstring(L, result.error_message.c_str());
    return 2;
}

/**
 * Lua binding for removing a directory and all its contents.
 * @param L The Lua state.
 * @return Number of return values (2: success boolean and error message).
 * Lua usage: success, error_message = lua_rmdir_all(path)
 *   - path: The directory path to remove.
 */
int lua_rmdir_all(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);

    RemovePathResult result = rmdir_all(path);

    lua_pushboolean(L, result.success);
    lua_pushstring(L, result.error_message.c_str());
    return 2;
}