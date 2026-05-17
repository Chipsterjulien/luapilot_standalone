#include "symlinkattr.hpp"
#include "lua_utils.hpp"
#include <unistd.h>
#include <cerrno>
#include <system_error>

int lua_symlinkattr(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 3)
    {
        return luaL_error(L, "Expected three arguments: path, owner (UID), and group (GID)");
    }
    if (!lua_isstring(L, 1))
    {
        return luaL_argerror(L, 1, "Expected a string as the first argument");
    }
    if (!lua_isinteger(L, 2))
    {
        return luaL_argerror(L, 2, "Expected an integer as the second argument (owner UID)");
    }
    if (!lua_isinteger(L, 3))
    {
        return luaL_argerror(L, 3, "Expected an integer as the third argument (group GID)");
    }

    const char *path = luaL_checkstring(L, 1);
    lua_Integer owner_raw = luaL_checkinteger(L, 2);
    lua_Integer group_raw = luaL_checkinteger(L, 3);

    if (owner_raw < 0 || group_raw < 0)
    {
        return push_fail(L, "UID and GID must be non-negative");
    }

    uid_t owner = static_cast<uid_t>(owner_raw);
    gid_t group = static_cast<gid_t>(group_raw);

    // lchown agit sur le lien symbolique lui-même, sans le suivre
    // (chown, lui, agirait sur la cible du lien).
    if (lchown(path, owner, group) != 0)
    {
        return push_fail(L, std::generic_category().message(errno));
    }

    return push_ok(L);
}