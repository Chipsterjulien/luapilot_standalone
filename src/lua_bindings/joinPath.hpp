#ifndef JOINPATH_HPP
#define JOINPATH_HPP

#include <lua.hpp>
#include <string>
#include <vector>
#include <optional>

/**
 * @brief Function to join path segments
 *
 * This function takes a vector of strings representing path segments
 * et joins them to form a complete path, handling separators correctly.
 *
 * @param segments Vector of strings representing the path segments
 * @return A std::optional<std::string> representing the complete joined path or an error message
 */
std::optional<std::string> join(const std::vector<std::string>& segments);

/**
 * @brief Retrieve segments from Lua stack
 *
 * This function retrieves segments from a Lua table or multiple string arguments.
 *
 * @param L Pointer to the Lua state
 * @param segments Reference to the vector where segments will be stored
 * @return Error message if any, otherwise an empty optional
 */
std::optional<std::string> get_segments(lua_State* L, std::vector<std::string>& segments);

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
int lua_joinPath(lua_State* L);

#endif // JOINPATH_HPP
