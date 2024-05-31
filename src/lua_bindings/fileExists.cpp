#include "lua_bindings/fileExists.hpp"
#include <filesystem>

namespace fs = std::filesystem;

bool fileExists(const std::string &path)
{
    return fs::exists(path) && fs::is_regular_file(path);
}

// Lua wrapper for the fileExists function
int lua_fileExists(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        return luaL_error(L, "Expected a string as the first argument");
    }

    const char *path = luaL_checkstring(L, 1);
    bool exists = fileExists(path);
    lua_pushboolean(L, exists);
    return 1;
}
