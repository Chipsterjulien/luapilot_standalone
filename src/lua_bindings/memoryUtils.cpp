#include "lua_bindings/memoryUtils.hpp"
#include <lua.hpp>

// Fonction Lua pour obtenir l'utilisation de la mémoire
int lua_getMemoryUsage(lua_State *L) {
    lua_gc(L, LUA_GCCOLLECT, 0); // Collecte les objets inutilisés (garbage collection)
    int memoryUsage = lua_gc(L, LUA_GCCOUNT, 0); // Obtient l'utilisation de la mémoire en kilo-octets
    lua_pushnumber(L, memoryUsage); // Pousse l'utilisation de la mémoire sur la pile Lua
    return 1;
}
