#ifndef MEMORY_UTILS_HPP
#define MEMORY_UTILS_HPP

#include <lua.hpp>

/**
 * Lua binding for getting memory usage.
 * @param L The Lua state.
 * @return Number of return values (1: memory usage in KB).
 * Lua usage: memoryUsage = lua_getMemoryUsage()
 */
int lua_getMemoryUsage(lua_State *L);

#endif // MEMORY_UTILS_HPP
