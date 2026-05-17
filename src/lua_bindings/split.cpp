#include "split.hpp"
#include <cstring>
#include <string>
#include <string_view>

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
int lua_split(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc < 1 || argc > 3) {
        return luaL_error(L, "Expected 1 to 3 arguments: string, optional delimiter, and optional max_splits");
    }

    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as the first argument");
    }
    const char* str = luaL_checkstring(L, 1);
    size_t str_len = std::strlen(str);

    char delimiter = ' ';
    bool has_delimiter = false;
    if (argc >= 2) {
        if (!lua_isstring(L, 2)) {
            return luaL_error(L, "Expected a string as the second argument");
        }
        const char* delim = luaL_checkstring(L, 2);
        size_t delim_len = lua_rawlen(L, 2);
        if (delim_len == 1) {
            delimiter = delim[0];
            has_delimiter = true;
        } else if (delim_len > 1) {
            return luaL_error(L, "Delimiter should be a single character or an empty string");
        }
    }

    int max_splits = -1;
    if (argc == 3) {
        if (!lua_isinteger(L, 3)) {
            return luaL_error(L, "Expected an integer as the third argument");
        }
        max_splits = luaL_checkinteger(L, 3);
        if (max_splits < -1) {
            return luaL_error(L, "max_splits should be -1 or greater");
        }
    }

    lua_newtable(L);
    int table_index = lua_gettop(L);

    if (!has_delimiter) {
        // Split by characters
        for (size_t i = 0; i < str_len; ++i) {
            lua_pushinteger(L, i + 1);
            lua_pushlstring(L, str + i, 1);
            lua_settable(L, table_index);
        }
    } else {
        std::string_view str_view(str, str_len);
        size_t start = 0;
        size_t end = 0;
        int splits = 0;
        int index = 1;

        while (end != std::string::npos) {
            end = str_view.find_first_of(delimiter, start);
            if (end == std::string::npos || (max_splits != -1 && splits >= max_splits)) {
                lua_pushinteger(L, index++);
                lua_pushlstring(L, str + start, str_len - start);
                lua_settable(L, table_index);
                break;
            } else {
                lua_pushinteger(L, index++);
                lua_pushlstring(L, str + start, end - start);
                lua_settable(L, table_index);
                start = end + 1;
                ++splits;
            }
        }
    }

    return 1;
}
