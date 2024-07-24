#ifndef SPLIT_HPP
#define SPLIT_HPP

#include <string>
#include <vector>
#include <sstream>
#include <lua.hpp>

/**
 * Split a string using a delimiter with a limit on the number of splits.
 * If no delimiter is provided, split the string into individual characters.
 * @param str The input string.
 * @param delimiter The delimiter character. If '\0', split into characters.
 * @param max_splits The maximum number of splits. If -1, no limit.
 * @return A vector of strings resulting from the split.
 */
std::vector<std::string> split(const std::string& str, char delimiter, int max_splits);

/**
 * Lua binding for splitting a string.
 * @param L The Lua state.
 * @return Number of return values (1: table of split strings).
 * Lua usage: parts = lua_split(str, delimiter, max_splits)
 *   - str: The input string to split.
 *   - delimiter: The character to split the string by (optional).
 *   - max_splits (optional): The maximum number of splits. Defaults to -1.
 */
int lua_split(lua_State* L);

#endif // SPLIT_HPP
