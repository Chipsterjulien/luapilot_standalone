#include "memoryUtils.hpp"
#include <lua.hpp>

/**
 * Lua binding for getting memory usage.
 * @param L The Lua state.
 * @return Number of return values (1: memory usage in KB).
 * Lua usage: memoryUsage = lua_getMemoryUsage()
 */
int lua_getMemoryUsage(lua_State *L) {
    lua_gc(L, LUA_GCCOLLECT, 0); // Collecte les objets inutilisés (garbage collection)
    int memoryUsage = lua_gc(L, LUA_GCCOUNT, 0); // Obtient l'utilisation de la mémoire en kilo-octets
    lua_pushnumber(L, memoryUsage); // Pousse l'utilisation de la mémoire sur la pile Lua
    return 1;
}