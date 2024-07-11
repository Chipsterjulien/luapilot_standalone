#include "lua_bindings/joinPath.hpp"
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
 * @return Number of return values on the Lua stack (1 on success, the joined path)
 */
int lua_joinPath(lua_State* L) {
    std::vector<std::string> segments;

    if (lua_istable(L, 1)) {
        lua_pushnil(L);  // First key
        while (lua_next(L, 1) != 0) {
            // Retrieve the value at the top of the Lua stack
            if (lua_isstring(L, -1)) {
                segments.push_back(lua_tostring(L, -1));
            } else {
                lua_pop(L, 1);
                return luaL_error(L, "Table must contain only strings");
            }
            lua_pop(L, 1);  // Remove the value, keep the key for the next iteration
        }
    } else {
        int n = lua_gettop(L); // Number of arguments
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
    return 1;  // One return value (the joined path)
}
