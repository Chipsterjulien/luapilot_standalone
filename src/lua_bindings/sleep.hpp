#ifndef SLEEP_HPP
#define SLEEP_HPP

#include <lua.hpp>

/**
 * Lua binding for the sleep function.
 * @param L The Lua state.
 * @return Number of return values (0).
 * Lua usage: lua_sleep(duration, unit)
 *   - duration: The amount of time to sleep.
 *   - unit: The unit of time (optional, default is seconds). Can be "s" for seconds, "ms" for milliseconds, "us" for microseconds.
 */
int lua_sleep(lua_State* L);

#endif // SLEEP_HPP
