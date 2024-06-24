#ifndef TOUCH_HPP
#define TOUCH_HPP

#include <string>
#include <lua.hpp>

/**
 * Create an empty file or update the timestamp of an existing file.
 * @param path The file path to touch.
 * @return A boolean indicating success or failure, and an error message if any.
 */
struct TouchResult {
    bool success;
    std::string error_message;
};

/**
 * Function to create or update a file.
 * @param path The file path.
 * @return A TouchResult struct containing the success status and an error message if any.
 */
TouchResult touch(const std::string& path);

/**
 * Lua binding for the touch function.
 * @param L The Lua state.
 * @return Number of return values (2: success boolean and error message).
 * Lua usage: success, error_message = lua_touch(path)
 *   - path: The file path to touch.
 */
int lua_touch(lua_State* L);

#endif // TOUCH_HPP
