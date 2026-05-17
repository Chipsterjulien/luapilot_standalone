#include "sha3_384.hpp"
#include "checksum_utils.hpp"
#include "openssl_utils.hpp"

std::optional<std::string> sha3_384sum(const std::string &path)
{
    return calculate_checksum(path, EVP_sha3_384());
}

int lua_sha3_384sum(lua_State *L)
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
    auto result = sha3_384sum(path);

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
