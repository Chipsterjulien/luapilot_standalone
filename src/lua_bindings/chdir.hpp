#ifndef CHDIR_HPP
#define CHDIR_HPP

#include <string>
#include <lua.hpp>

// Struct to hold the result of chdir function
struct ChdirResult {
    bool success;
    std::string error_message;
};

/**
 * Change the current working directory.
 * @param path The directory path to change to.
 * @return A ChdirResult struct containing the success status and an error message if any.
 */
ChdirResult chdir(const std::string& path);

/**
 * Lua binding for changing the current working directory.
 * @param L The Lua state.
 * @return Number of return values (2: success boolean and error message).
 * Lua usage: success, error_message = lua_chdir(path)
 *   - path: The directory path to change to.
 */
int lua_chdir(lua_State* L);

#endif // CHDIR_HPP