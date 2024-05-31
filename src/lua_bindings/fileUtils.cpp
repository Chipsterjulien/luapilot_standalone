#include "lua_bindings/fileUtils.hpp"
#include <filesystem>

namespace fs = std::filesystem;


std::string getBasename(const std::string &fullPath)
{
    return fs::path(fullPath).filename().string();
}

std::string getExtension(const std::string &fullPath)
{
    return fs::path(fullPath).extension().string();
}

std::string getFilename(const std::string &fullPath)
{
    return fs::path(fullPath).stem().string();
}

std::string getPath(const std::string &fullPath)
{
    return fs::path(fullPath).parent_path().string();
}


int lua_getBasename(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        return luaL_error(L, "Expected a string as the first argument");
    }

    const char *path = luaL_checkstring(L, 1);
    std::string basename = getBasename(path);
    lua_pushstring(L, basename.c_str());
    return 1;
}

int lua_getExtension(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        return luaL_error(L, "Expected a string as the first argument");
    }

    const char *path = luaL_checkstring(L, 1);
    std::string extension = getExtension(path);
    lua_pushstring(L, extension.c_str());
    return 1;
}

int lua_getFilename(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        return luaL_error(L, "Expected a string as the first argument");
    }

    const char *path = luaL_checkstring(L, 1);
    std::string filename = getFilename(path);
    lua_pushstring(L, filename.c_str());
    return 1;
}

// Lua wrapper for the getPath function
int lua_getPath(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        return luaL_error(L, "Expected a string as the first argument");
    }

    const char *path = luaL_checkstring(L, 1);
    std::string p = getPath(path);
    lua_pushfstring(L, p.c_str());
    return 1;
}