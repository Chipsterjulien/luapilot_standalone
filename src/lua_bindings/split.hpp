#ifndef SPLIT_HPP
#define SPLIT_HPP

#include <lua.hpp>

/**
 * @brief Splits a given string using a specified delimiter and inserts the substrings into a Lua table.
 *
 * This function expects up to three arguments from the Lua stack:
 * - The string to be split.
 * - An optional delimiter (default is space ' ').
 * - An optional maximum number of splits (default is no limit).
 *
 * If the delimiter is an empty string, the function splits the input string into individual characters.
 * The resulting substrings are pushed into a Lua table which is then returned.
 *
 * @param L The Lua state.
 * @return int The number of results to be returned to Lua (always 1, the table of substrings).
 *
 * @throws luaL_error If the arguments are invalid.
 */
int lua_split(lua_State* L);

#endif // SPLIT_HPP
