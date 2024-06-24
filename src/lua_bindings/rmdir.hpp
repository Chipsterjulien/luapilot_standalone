#ifndef RMDIR_HPP
#define RMDIR_HPP

#include <string>
#include <lua.hpp>

// Struct to hold the result of remove_path function
struct RemovePathResult {
    bool success;
    std::string error_message;
};

/**
 * Remove a file or an empty directory.
 * @param path The file or directory path to remove.
 * @return A RemovePathResult struct containing the success status and an error message if any.
 */
RemovePathResult rmdir(const std::string& path);

/**
 * Remove a directory and all its contents.
 * @param path The directory path to remove.
 * @return A RemovePathResult struct containing the success status and an error message if any.
 */
RemovePathResult rmdir_all(const std::string& path);

/**
 * Lua binding for removing a file or an empty directory.
 * @param L The Lua state.
 * @return Number of return values (2: success boolean and error message).
 * Lua usage: success, error_message = lua_rmdir(path)
 *   - path: The file or directory path to remove.
 */
int lua_rmdir(lua_State* L);

/**
 * Lua binding for removing a directory and all its contents.
 * @param L The Lua state.
 * @return Number of return values (2: success boolean and error message).
 * Lua usage: success, error_message = lua_rmdir_all(path)
 *   - path: The directory path to remove.
 */
int lua_rmdir_all(lua_State* L);

#endif // RMDIR_HPP
