#include "lua_bindings/split.hpp"
#include <sstream>

// Function to split a string by a delimiter with a limit on the number of splits
std::vector<std::string> split(const std::string &str, char delimiter, int max_splits)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    int splits = 0;

    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
        if (max_splits != -1 && ++splits >= max_splits)
        {
            // Add the remaining part of the string as the last token
            std::string remaining;
            std::getline(tokenStream, remaining);
            tokens.push_back(remaining);
            break;
        }
    }

    return tokens;
}

// Lua wrapper for the split function
int split(lua_State *L)
{
    // Check the number of arguments
    int argc = lua_gettop(L);
    if (argc < 2 || argc > 3)
    {
        return luaL_error(L, "Expected 2 or 3 arguments: string, delimiter, and optional max_splits");
    }

    // Check and get the first argument
    if (!lua_isstring(L, 1))
    {
        return luaL_error(L, "Expected a string as the first argument");
    }
    const char *str = luaL_checkstring(L, 1);

    // Check and get the second argument
    if (!lua_isstring(L, 2))
    {
        return luaL_error(L, "Expected a string as the second argument");
    }
    std::string delim = luaL_checkstring(L, 2);
    if (delim.length() != 1)
    {
        return luaL_error(L, "Delimiter should be a single character");
    }

    // Optional third argument: max_splits
    int max_splits = -1; // Default value
    if (argc == 3)
    {
        if (!lua_isinteger(L, 3))
        {
            return luaL_error(L, "Expected an integer as the third argument");
        }
        max_splits = luaL_checkinteger(L, 3);
    }

    // Perform the split operation (assuming a function split is defined elsewhere)
    std::vector<std::string> tokens = split(str, delim[0], max_splits);

    // Create a new table on the Lua stack
    lua_newtable(L);

    // Push the tokens into the table
    int index = 1;
    for (const std::string &token : tokens)
    {
        lua_pushnumber(L, index++);
        lua_pushstring(L, token.c_str());
        lua_settable(L, -3);
    }

    // Return the table
    return 1;
}
