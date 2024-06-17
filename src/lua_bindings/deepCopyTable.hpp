#ifndef DEEP_COPY_TABLE_HPP
#define DEEP_COPY_TABLE_HPP

#include <lua.hpp>

// Fonction pour copier une table lua
void deepCopyTable(lua_State *L, int srcIndex, int destIndex, int &nextIndex);

// Fonction pour faire une copie profonde d'une table Lua
int lua_deepCopyTable(lua_State *L);

#endif