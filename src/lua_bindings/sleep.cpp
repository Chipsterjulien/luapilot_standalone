#include "sleep.hpp"
#include <thread>
#include <chrono>
#include <stdexcept>

/**
 * Converts a string to a std::chrono::duration based on the unit.
 * @param duration The duration value.
 * @param unit The unit of time ("s" for seconds, "ms" for milliseconds, "us" for microseconds).
 * @return The corresponding std::chrono::duration.
 */
std::chrono::duration<double> convert_to_duration(double duration, const std::string& unit) {
    if (unit == "s") {
        return std::chrono::duration<double>(duration);
    } else if (unit == "ms") {
        return std::chrono::duration<double, std::milli>(duration);
    } else if (unit == "us") {
        return std::chrono::duration<double, std::micro>(duration);
    } else {
        throw std::invalid_argument("Invalid time unit");
    }
}

/**
 * Lua binding for the sleep function.
 * @param L The Lua state.
 * @return Number of return values (0).
 * Lua usage: lua_sleep(duration, unit)
 *   - duration: The amount of time to sleep.
 *   - unit: The unit of time (optional, default is seconds). Can be "s" for seconds, "ms" for milliseconds, "us" for microseconds.
 */
int lua_sleep(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc < 1 || argc > 2) {
        return luaL_error(L, "Expected one or two arguments: duration and optional unit");
    }

    if (!lua_isnumber(L, 1)) {
        return luaL_argerror(L, 1, "Expected a number as the first argument for duration");
    }

    double duration = lua_tonumber(L, 1);
    if (duration < 0) {
        return luaL_argerror(L, 1, "Duration must be non-negative");
    }

    std::string unit = "s";
    if (argc == 2) {
        if (!lua_isstring(L, 2)) {
            return luaL_argerror(L, 2, "Expected a string as the second argument for unit");
        }
        unit = lua_tostring(L, 2);
    }

    try {
        auto duration_chrono = convert_to_duration(duration, unit);
        std::this_thread::sleep_for(duration_chrono);
    } catch (const std::invalid_argument& e) {
        return luaL_argerror(L, 2, e.what());
    }

    return 0;
}
