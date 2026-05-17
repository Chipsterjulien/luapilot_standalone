#include "md5.hpp"
#include "checksum_utils.hpp"
#include "openssl_utils.hpp"
#include <openssl/evp.h>

std::optional<std::string> md5sum(const std::string &path)
{
    return calculate_checksum(path, EVP_md5());
}

int lua_md5sum(lua_State *L)
{
    if (lua_gettop(L) != 1 || !lua_isstring(L, 1))
    {
        return luaL_error(L, "Expected one string argument");
    }

    const char *path = luaL_checkstring(L, 1);
    auto result = md5sum(path);

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