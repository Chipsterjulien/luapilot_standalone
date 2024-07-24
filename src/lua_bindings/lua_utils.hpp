#ifndef LUA_UTILS_HPP
#define LUA_UTILS_HPP

#include <lua.hpp>
#include <string>

/**
 * @brief Utility function to push an error message to Lua stack.
 *
 * @param L The Lua state.
 * @param message The error message to push.
 * @return int Always returns 1 to indicate one return value on the Lua stack.
 */
inline int push_error(lua_State* L, const std::string& message) {
    lua_pushstring(L, message.c_str());
    return 1;
}

#endif // LUA_UTILS_HPP
