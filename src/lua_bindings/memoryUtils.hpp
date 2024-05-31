#ifndef MEMORY_UTILS_HPP
#define MEMORY_UTILS_HPP

#include <lua.hpp>

// Fonction pour obtenir l'utilisation de la mémoire
int lua_getMemoryUsage(lua_State *L);

#endif // MEMORY_UTILS_HPP
