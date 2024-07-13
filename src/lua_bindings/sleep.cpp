#include "sleep.hpp"
#include <thread>
#include <chrono>

/**
 * Lua binding for the sleep function.
 * @param L The Lua state.
 * @return Number of return values (0).
 * Lua usage: lua_sleep(seconds)
 *   - seconds: The number of seconds to sleep.
 */
int lua_sleep(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument: number of seconds to sleep");
    }

    if (!lua_isnumber(L, 1)) {
        return luaL_error(L, "Expected a number as the first argument");
    }

    int seconds = luaL_checkinteger(L, 1);
    if (seconds < 0) {
        return luaL_error(L, "Expected a non-negative number of seconds");
    }

    std::this_thread::sleep_for(std::chrono::seconds(seconds));

    return 0;
}