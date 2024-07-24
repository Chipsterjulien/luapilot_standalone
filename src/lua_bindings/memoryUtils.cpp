#include "memoryUtils.hpp"
#include <lua.hpp>

/**
 * @brief Lua binding for getting memory usage.
 *
 * This function collects garbage and returns the memory usage of the Lua state in bytes.
 *
 * @param L The Lua state.
 * @return int Number of return values (1: memory usage in bytes).
 * @note Lua usage: memoryUsedBytes = lua_getMemoryUsage()
 */
int lua_getMemoryUsage(lua_State *L) {
    lua_gc(L, LUA_GCCOLLECT, 0); // Perform garbage collection
    int memoryUsedUnits = lua_gc(L, LUA_GCCOUNT, 0); // Get memory used in Lua word units
    int memoryUsedBytes = memoryUsedUnits * sizeof(lua_Integer); // Calculate memory used in bytes

    lua_pushnumber(L, memoryUsedBytes);
    return 1;
}

/**
 * @brief Lua binding for getting detailed memory usage.
 *
 * This function collects garbage and returns the memory usage and an estimation
 * of the total memory in bytes.
 *
 * @param L The Lua state.
 * @return int Number of return values (2: memory usage in bytes, estimated total memory in bytes).
 * @note Lua usage: memoryUsedBytes, memoryTotalBytes = lua_getDetailedMemoryUsage()
 */
int lua_getDetailedMemoryUsage(lua_State *L) {
    lua_gc(L, LUA_GCCOLLECT, 0); // Perform garbage collection
    int memoryUsedUnits = lua_gc(L, LUA_GCCOUNT, 0); // Get memory used in Lua word units
    int memoryUsedBytes = memoryUsedUnits * sizeof(lua_Integer); // Calculate memory used in bytes
    int memoryFragmentBytes = lua_gc(L, LUA_GCCOUNTB, 0); // Get memory fragment in bytes

    // Estimate total memory (may not be completely accurate)
    int memoryTotalBytes = memoryUsedBytes + memoryFragmentBytes;

    lua_pushnumber(L, memoryUsedBytes);
    lua_pushnumber(L, memoryTotalBytes);
    return 2;
}
