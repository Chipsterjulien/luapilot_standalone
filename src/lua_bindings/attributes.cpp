#include "attributes.hpp"
#include "lua_utils.hpp"
#include <filesystem>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>

namespace fs = std::filesystem;

/**
 * @brief Pushes a Lua table {mode, owner, group} on the stack.
 */
static void push_attributes(lua_State *L, fs::perms perms, uid_t owner, gid_t group)
{
    lua_newtable(L);

    lua_pushstring(L, "mode");
    lua_pushinteger(L, static_cast<int>(perms) & 0777);
    lua_settable(L, -3);

    lua_pushstring(L, "owner");
    lua_pushinteger(L, owner);
    lua_settable(L, -3);

    lua_pushstring(L, "group");
    lua_pushinteger(L, group);
    lua_settable(L, -3);
}

/**
 * @brief Changes the owner, group, and optionally the permissions of a file or directory.
 * @return std::nullopt on success, an error message otherwise.
 */
static std::optional<std::string> set_attributes(const std::filesystem::path &path,
                                                 uid_t owner, gid_t group,
                                                 std::optional<fs::perms> mode)
{
    if (chown(path.c_str(), owner, group) != 0)
    {
        return std::generic_category().message(errno);
    }

    if (mode)
    {
        std::error_code ec;
        fs::permissions(path, *mode, ec);
        if (ec)
        {
            return ec.message();
        }
    }

    return std::nullopt;
}

int lua_setattr(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc < 3 || argc > 4)
    {
        return luaL_error(L, "Expected three or four arguments");
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

    std::optional<fs::perms> mode = std::nullopt;
    if (argc == 4)
    {
        mode = static_cast<fs::perms>(luaL_checkinteger(L, 4));
    }

    return push_action_result(L, set_attributes(path, owner, group, mode));
}

int lua_getattr(lua_State *L)
{
    if (lua_gettop(L) != 1 || !lua_isstring(L, 1))
    {
        return luaL_error(L, "Expected one string argument");
    }

    const char *path = luaL_checkstring(L, 1);

    struct stat fileStat;
    if (stat(path, &fileStat) != 0)
    {
        return push_fail(L, std::generic_category().message(errno));
    }

    std::error_code ec;
    fs::perms perms = fs::status(path, ec).permissions();
    if (ec)
    {
        return push_fail(L, ec.message());
    }

    push_attributes(L, perms, fileStat.st_uid, fileStat.st_gid);
    lua_pushnil(L);
    return 2;
}