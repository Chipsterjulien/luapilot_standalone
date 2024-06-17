#include "lua_bindings/fileExists.hpp"
#include <filesystem>

namespace fs = std::filesystem;

bool fileExists(const std::string &path) {
    return fs::exists(path) && fs::is_regular_file(path);
}

// Interface Lua pour la fonction fileExists
int lua_fileExists(lua_State *L) {
    // Vérifier qu'il y a un argument passé
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Vérifier que l'argument est une chaîne de caractères
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    // Récupérer le chemin du fichier à partir des arguments de lua
    const char *path = luaL_checkstring(L, 1);

    // Vérifier si le fichier existe
    bool exists = fileExists(path);

    // Pousser le résultat sur la pile lua
    lua_pushboolean(L, exists);

    return 1;
}
