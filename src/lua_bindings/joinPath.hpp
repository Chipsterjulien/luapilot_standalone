#ifndef JOINPATH_HPP
#define JOINPATH_HPP

#include <lua.hpp>
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
std::string join(const std::vector<std::string>& segments);

/**
 * @brief Lua-accessible function to join path segments
 *
 * This function is called from Lua to join multiple path segments.
 * It can take either a Lua table containing strings representing path segments
 * or multiple string arguments representing path segments.
 * It returns a single string representing the complete path.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (2 on success: the joined path and nil, or nil and an error message)
 */
int lua_joinPath(lua_State* L);

#endif // JOINPATH_HPP
