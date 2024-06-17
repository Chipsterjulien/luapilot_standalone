#include "lua_bindings/fileSize.hpp"
#include <filesystem>
#include <cstdint>
#include <stdexcept>

namespace fs = std::filesystem;

// Fonction pour obtenir la taille d'un fichier en octets
bool fileSize(const std::string &path, std::uint64_t &size) {
    // Vérifie si le chemin existe et est un fichier régulier
    if (fs::exists(path) && fs::is_regular_file(path)) {
        // Obtient la taille du fichier et la stocke dans la variable size
        size = static_cast<std::uint64_t>(fs::file_size(path));
        return true; // Retourne true si la taille a été obtenue avec succès
    }
    return false; // Retourne false si le fichier n'existe pas ou n'est pas un fichier régulier
}

// Fonction Lua pour obtenir la taille d'un fichier
int lua_fileSize(lua_State *L) {
    // Obtient le nombre d'arguments passés à la fonction Lua
    int argc = lua_gettop(L);
    // Vérifie qu'un seul argument est passé
    if (argc != 1) {
        return luaL_error(L, "Expected one argument"); // Retourne une erreur si ce n'est pas le cas
    }

    // Vérifie que l'argument est une chaîne de caractères
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument"); // Retourne une erreur si ce n'est pas le cas
    }

    // Récupère le chemin du fichier à partir de l'argument Lua
    const char *path = luaL_checkstring(L, 1);
    std::uint64_t size;
    // Appelle la fonction fileSize pour obtenir la taille du fichier
    bool success = fileSize(path, size);

    // Vérifie si l'obtention de la taille du fichier a échoué
    if (!success) {
        return luaL_error(L, "File does not exist or is not a regular file"); // Retourne une erreur
    }

    // Pousse la taille du fichier sur la pile Lua en tant qu'entier
    lua_pushinteger(L, static_cast<lua_Integer>(size));
    return 1;
}
