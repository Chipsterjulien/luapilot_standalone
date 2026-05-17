#include "blake2b.hpp"
#include "checksum_utils.hpp"
#include "openssl_utils.hpp"

std::optional<std::string> blake2b512sum(const std::string &path)
{
    return calculate_checksum(path, EVP_blake2b512());
}

int lua_blake2b512sum(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1)
    {
        return luaL_error(L, "Expected one argument");
    }

    if (!lua_isstring(L, 1))
    {
        return luaL_error(L, "Expected a string as argument");
    }

    const char *path = luaL_checkstring(L, 1);
    auto result = blake2b512sum(path);

    if (result.has_value())
    {
        lua_pushstring(L, result->c_str());
        lua_pushnil(L);
    }
    else
    {
        lua_pushnil(L);
        lua_pushstring(L, get_openssl_error().c_str());
    }
    return 2;
}
