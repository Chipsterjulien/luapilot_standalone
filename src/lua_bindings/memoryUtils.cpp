#include "lua_bindings/memoryUtils.hpp"
#include <lua.hpp>

int lua_getMemoryUsage(lua_State *L)
{
    lua_gc(L, LUA_GCCOLLECT, 0); // Collect garbage
    int memoryUsage = lua_gc(L, LUA_GCCOUNT, 0);
    lua_pushnumber(L, memoryUsage);
    return 1;
}
