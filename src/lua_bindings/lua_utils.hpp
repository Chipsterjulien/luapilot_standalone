#ifndef LUA_UTILS_HPP
#define LUA_UTILS_HPP

#include <lua.hpp>
#include <optional>
#include <string>
#include <string_view>

/**
 * @brief Pushes a single error message on the stack (legacy 1-return).
 * @return 1
 */
inline int push_error(lua_State *L, const std::string &message)
{
    lua_pushstring(L, message.c_str());
    return 1;
}

/**
 * @brief Pushes (true, nil) on the stack for an action that succeeded.
 * @return 2
 */
inline int push_ok(lua_State *L)
{
    lua_pushboolean(L, 1);
    lua_pushnil(L);
    return 2;
}

/**
 * @brief Pushes (nil, "err") on the stack for any failure.
 * @return 2
 */
inline int push_fail(lua_State *L, std::string_view message)
{
    lua_pushnil(L);
    lua_pushlstring(L, message.data(), message.size());
    return 2;
}

/**
 * @brief Convenience: takes an optional error from a C++ function and converts to (ok, err) Lua return.
 *        If `error` has a value, returns (nil, *error). Otherwise returns (true, nil).
 */
inline int push_action_result(lua_State *L, const std::optional<std::string> &error)
{
    if (error)
    {
        return push_fail(L, *error);
    }
    return push_ok(L);
}

#endif // LUA_UTILS_HPP
