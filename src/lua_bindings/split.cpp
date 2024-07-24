#include "split.hpp"
#include <stdexcept>

/**
 * Split a string using a delimiter with a limit on the number of splits.
 * If no delimiter is provided, split the string into individual characters.
 * @param str The input string.
 * @param delimiter The delimiter character. If '\0', split into characters.
 * @param max_splits The maximum number of splits. If -1, no limit.
 * @return A vector of strings resulting from the split.
 */
std::vector<std::string> split(const std::string& str, char delimiter, int max_splits) {
    std::vector<std::string> tokens;
    if (str.empty()) {
        return tokens;
    }

    if (delimiter == '\0') {
        // Split into individual characters
        tokens.reserve(str.size()); // Reserve space to avoid multiple allocations
        for (char ch : str) {
            tokens.emplace_back(1, ch); // More efficient than tokens.push_back(std::string(1, ch))
        }
        return tokens;
    }

    std::istringstream tokenStream(str);
    std::string token;
    int splits = 0;

    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
        if (max_splits != -1 && ++splits >= max_splits) {
            std::string remaining;
            std::getline(tokenStream, remaining);
            tokens.push_back(remaining);
            break;
        }
    }

    return tokens;
}

/**
 * Lua binding for splitting a string.
 * @param L The Lua state.
 * @return Number of return values (1: table of split strings).
 * Lua usage: parts = lua_split(str, delimiter, max_splits)
 *   - str: The input string to split.
 *   - delimiter: The character to split the string by (optional).
 *   - max_splits (optional): The maximum number of splits. Defaults to -1.
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

    char delimiter = '\0'; // Default to character by character split
    if (argc >= 2) {
        if (!lua_isstring(L, 2)) {
            return luaL_error(L, "Expected a string as the second argument");
        }
        std::string delim = luaL_checkstring(L, 2);
        if (delim.length() > 1) {
            return luaL_error(L, "Delimiter should be a single character or an empty string");
        }
        delimiter = delim.empty() ? '\0' : delim[0];
    }

    int max_splits = -1; // Default to no limit
    if (argc == 3) {
        if (!lua_isinteger(L, 3)) {
            return luaL_error(L, "Expected an integer as the third argument");
        }
        max_splits = luaL_checkinteger(L, 3);
        if (max_splits < -1) {
            return luaL_error(L, "max_splits should be -1 or greater");
        }
    }

    std::vector<std::string> tokens;
    try {
        tokens = split(str, delimiter, max_splits);
    } catch (const std::exception& e) {
        return luaL_error(L, e.what());
    }

    lua_newtable(L);

    int index = 1;
    for (const std::string& token : tokens) {
        lua_pushinteger(L, index++);
        lua_pushstring(L, token.c_str());
        lua_settable(L, -3);
    }

    return 1;
}
