#include "joinPath.hpp"
#include <string>
#include <vector>

/**
 * @brief Function to join path segments
 *
 * This function takes a vector of strings representing path segments
 * and joins them to form a complete path, handling separators correctly.
 *
 * @param segments Vector of strings representing the path segments
 * @return A string representing the complete joined path
 */
std::string join(const std::vector<std::string>& segments) {
    std::string path;
    for (const auto& segment : segments) {
        if (!segment.empty()) {
            std::string temp_segment = segment;  // Use a temporary variable to hold the segment
            if (!path.empty() && path.back() != '/' && segment.front() != '/') {
                path += '/'; // Add separator if needed
            } else if (!path.empty() && path.back() == '/' && segment.front() == '/') {
                temp_segment = segment.substr(1); // Remove leading separator from segment
            }
            path += temp_segment;
        }
    }
    return path;
}

/**
 * @brief Lua-accessible function to join path segments
 *
 * This function is called from Lua and uses the join function to join path segments.
 * It can take either a Lua table containing strings or multiple string arguments.
 * If the argument is not a table or if the table contains non-string elements, a Lua error is raised.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (2: fullpath and error message)
 */
int lua_joinPath(lua_State* L) {
    std::vector<std::string> segments;

    if (lua_istable(L, 1)) {
        // Check that the table contains at least two elements
        if (lua_rawlen(L, 1) < 2) {
            return luaL_error(L, "Table must contain at least two strings");
        }

        lua_pushnil(L);  // First key
        while (lua_next(L, 1) != 0) {
            // Retrieve the value at the top of the Lua stack
            if (lua_isstring(L, -1)) {
                segments.push_back(lua_tostring(L, -1));
            } else {
                return luaL_error(L, "Table must contain at least two strings");
            }
            lua_pop(L, 1);  // Remove the value, keep the key for the next iteration
        }
    } else {
        int n = lua_gettop(L); // Number of arguments
        // Check that there are at least two string parameters
        if (n < 2) {
            return luaL_error(L, "Expected at least two string arguments");
        }

        for (int i = 1; i <= n; ++i) {
            if (lua_isstring(L, i)) {
                segments.push_back(lua_tostring(L, i));
            } else {
                return luaL_error(L, "All arguments must be strings");
            }
        }
    }

    std::string result = join(segments);
    lua_pushstring(L, result.c_str());
    lua_pushnil(L); // No error
    return 2;  // Two return values (the joined path and nil)
}
