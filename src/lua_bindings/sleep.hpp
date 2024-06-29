#ifndef SLEEP_HPP
#define SLEEP_HPP

#include <lua.hpp>

/**
 * Lua binding for the sleep function.
 * @param L The Lua state.
 * @return Number of return values (0).
 * Lua usage: lua_sleep(seconds)
 *   - seconds: The number of seconds to sleep.
 */
int lua_sleep(lua_State* L);

#endif // SLEEP_HPP
