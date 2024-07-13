#ifndef TOUCH_HPP
#define TOUCH_HPP

#include <string>
#include <lua.hpp>

/**
 * Function to create or update a file.
 * @param path The file path.
 * @return A string containing an error message if any, or an empty string if successful.
 */
std::string touch(const std::string& path);

/**
 * Lua binding for the touch function.
 * @param L The Lua state.
 * @return Number of return values (1: error message or nil).
 * Lua usage: error_message = lua_touch(path)
 *   - path: The file path to touch.
 */
int lua_touch(lua_State* L);

#endif // TOUCH_HPP
