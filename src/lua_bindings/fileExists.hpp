#ifndef FILE_EXISTS_HPP
#define FILE_EXISTS_HPP

#include <string>
#include <lua.hpp>

// Vérifie si un fichier existe à l'emplacement donné
bool fileExists(const std::string &path);
// Interface Lua pour la fonction fileExists
int lua_fileExists(lua_State *L);

#endif // FILE_EXISTS_HPP
