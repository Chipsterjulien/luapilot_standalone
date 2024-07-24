#include "joinPath.hpp"
#include <string>
#include <vector>
#include <optional>

/**
 * @brief Retrieve segments from Lua stack
 *
 * This function retrieves segments from a Lua table or multiple string arguments.
 *
 * @param L Pointer to the Lua state
 * @param segments Reference to the vector where segments will be stored
 * @return Error message if any, otherwise an empty optional
 */
std::optional<std::string> get_segments(lua_State* L, std::vector<std::string>& segments) {
    if (lua_istable(L, 1)) {
        if (lua_rawlen(L, 1) < 2) {
            return "Table must contain at least two strings";
        }

        lua_pushnil(L);
        while (lua_next(L, 1) != 0) {
            if (lua_isstring(L, -1)) {
                size_t len;
                const char* segment = lua_tolstring(L, -1, &len);
                if (len == 0) {
                    return "Segments cannot be empty";
                }
                segments.emplace_back(segment, len);
            } else {
                return "Table contains non-string elements";
            }
            lua_pop(L, 1);
        }
    } else {
        int n = lua_gettop(L);
        if (n < 2) {
            return "Expected at least two string arguments";
        }

        segments.reserve(n); // Reserve space for performance
        for (int i = 1; i <= n; ++i) {
            if (lua_isstring(L, i)) {
                size_t len;
                const char* segment = lua_tolstring(L, i, &len);
                if (len == 0) {
                    return "Segments cannot be empty";
                }
                segments.emplace_back(segment, len);
            } else {
                return "All arguments must be strings";
            }
        }
    }

    return std::nullopt;
}

/**
 * @brief Function to join path segments
 *
 * This function takes a vector of strings representing path segments
 * and joins them to form a complete path, handling separators correctly.
 *
 * @param segments Vector of strings representing the path segments
 * @return A std::optional<std::string> representing the complete joined path or an error message
 */
std::optional<std::string> join(const std::vector<std::string>& segments) {
    if (segments.empty()) {
        return "No segments provided";
    }

    std::string path = segments.front();

    for (size_t i = 1; i < segments.size(); ++i) {
        const auto& segment = segments[i];
        if (segment.empty()) {
            return "Empty segment found";
        }
        if (path.back() != '/' && segment.front() != '/') {
            path += '/';
        } else if (path.back() == '/' && segment.front() == '/') {
            path.pop_back();
        }
        path += segment;
    }

    return path;
}

/**
 * @brief Lua-accessible function to join path segments
 *
 * This function is called from Lua to join multiple path segments.
 * It can take either a Lua table containing strings representing path segments
 * or multiple string arguments representing path segments.
 * It returns a single string representing the complete path.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1 on success: the joined path, or nil and an error message on failure)
 */
int lua_joinPath(lua_State* L) {
    std::vector<std::string> segments;
    auto error = get_segments(L, segments);

    if (error) {
        lua_pushnil(L);
        lua_pushstring(L, error->c_str());
        return 2; // Return nil and error message
    }

    auto result = join(segments);
    if (!result) {
        lua_pushnil(L);
        lua_pushstring(L, result.value_or("Unknown error").c_str());
        return 2; // Return nil and error message
    }

    lua_pushstring(L, result->c_str());
    return 1; // Return the joined path
}
