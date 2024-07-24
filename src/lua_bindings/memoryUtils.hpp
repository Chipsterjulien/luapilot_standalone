#ifndef MEMORY_UTILS_HPP
#define MEMORY_UTILS_HPP

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
int lua_getMemoryUsage(lua_State *L);

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
int lua_getDetailedMemoryUsage(lua_State *L);

#endif // MEMORY_UTILS_HPP
