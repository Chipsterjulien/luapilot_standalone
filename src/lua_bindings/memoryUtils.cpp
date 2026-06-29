#include "memoryUtils.hpp"
#include <lua.hpp>

// CORRECTIF (post-revue Gemini) : la formule précédente était fausse.
// L'API Lua C définit :
//   LUA_GCCOUNT  -> nombre entier de Kilooctets utilisés
//   LUA_GCCOUNTB -> reste en octets (de la division par 1024)
// (Cf. Reference Manual Lua 5.x, section "Garbage Collection".)
//
// Le code précédent multipliait LUA_GCCOUNT par sizeof(lua_Integer)
// (8 octets sur 64 bits), donc renvoyait une valeur ~125× trop
// petite. La formule correcte est : (kb * 1024) + remainder.

/**
 * @brief Lua binding for getting memory usage in bytes.
 *
 * Performs a full GC collection and returns the current memory used
 * by the Lua state in bytes.
 *
 * @param L The Lua state.
 * @return int Number of return values (1: memory usage in bytes).
 * @note Lua usage: bytes = babet.getMemoryUsage()
 */
int lua_getMemoryUsage(lua_State *L)
{
    lua_gc(L, LUA_GCCOLLECT, 0);
    int kb = lua_gc(L, LUA_GCCOUNT, 0);         // Kilooctets entiers
    int remainder = lua_gc(L, LUA_GCCOUNTB, 0); // Reste en octets
    long long memoryUsedBytes =
        (long long)kb * 1024LL + (long long)remainder;

    lua_pushinteger(L, (lua_Integer)memoryUsedBytes);
    return 1;
}

/**
 * @brief Lua binding for getting detailed memory usage.
 *
 * Performs a full GC collection and returns two values:
 *   1. memory used in bytes (same as getMemoryUsage)
 *   2. an "estimated total" which is currently the same value, since
 *      Lua does not expose a separate "fragmentation" or "allocated
 *      but unused" metric. Kept as two returns for API stability.
 *
 * @param L The Lua state.
 * @return int Number of return values (2: bytes, bytes).
 * @note Lua usage: bytes, total = babet.getDetailedMemoryUsage()
 */
int lua_getDetailedMemoryUsage(lua_State *L)
{
    lua_gc(L, LUA_GCCOLLECT, 0);
    int kb = lua_gc(L, LUA_GCCOUNT, 0);
    int remainder = lua_gc(L, LUA_GCCOUNTB, 0);
    long long memoryUsedBytes =
        (long long)kb * 1024LL + (long long)remainder;

    // NOTE post-revue Gemini : l'ancien code traitait LUA_GCCOUNTB
    // comme un "fragment" séparable, ce qui était une erreur de
    // lecture de l'API. LUA_GCCOUNTB n'est PAS de la fragmentation,
    // c'est juste le reste en octets de COUNT (qui est en Ko). On
    // garde les deux returns pour ne pas casser les appelants
    // existants, mais les deux valeurs sont aujourd'hui identiques.
    lua_pushinteger(L, (lua_Integer)memoryUsedBytes);
    lua_pushinteger(L, (lua_Integer)memoryUsedBytes);
    return 2;
}
