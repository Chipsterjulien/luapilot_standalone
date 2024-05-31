#ifndef LUA_TABLE_COPY_HPP
#define LUA_TABLE_COPY_HPP

#include <lua.hpp>

// Fonction pour faire une copie superficielle d'une table Lua
int lua_shallowCopy(lua_State *L);

// Fonction pour faire une copie profonde d'une table Lua
int lua_deepCopy(lua_State *L);

#endif // LUA_TABLE_COPY_HPP
