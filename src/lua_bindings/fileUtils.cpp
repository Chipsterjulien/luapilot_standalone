#include "lua_bindings/fileUtils.hpp"
#include <filesystem>

namespace fs = std::filesystem;

// Fonction pour obtenir le nom de base (nom du fichier avec extension) à partir d'un chemin complet
std::string getBasename(const std::string &fullPath) {
    return fs::path(fullPath).filename().string();
}

// Fonction pour obtenir l'extension du fichier à partir d'un chemin complet
std::string getExtension(const std::string &fullPath) {
    return fs::path(fullPath).extension().string();
}

// Fonction pour obtenir le nom du fichier sans extension à partir d'un chemin complet
std::string getFilename(const std::string &fullPath) {
    return fs::path(fullPath).stem().string();
}

// Fonction pour obtenir le chemin parent à partir d'un chemin complet
std::string getPath(const std::string &fullPath) {
    return fs::path(fullPath).parent_path().string();
}

// Fonction Lua pour obtenir le nom de base
int lua_getBasename(lua_State *L) {
    // Obtient le nombre d'arguments passés à la fonction Lua
    int argc = lua_gettop(L);
    // Vérifie qu'un seul argument est passé
    if (argc != 1) {
        return luaL_error(L, "Expected one argument"); // Retourne une erreur si ce n'est pas le cas
    }

    // Vérifie que le premier argument est une chaîne de caractères
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as the first argument");
    }

    // Récupère le chemin du fichier à partir de l'argument Lua
    const char *path = luaL_checkstring(L, 1);
    // Obtient le nom de base à partir du chemin complet
    std::string basename = getBasename(path);
    // Pousse le nom de base sur la pile Lua en tant que chaîne
    lua_pushstring(L, basename.c_str());
    return 1; // Retourne 1 pour indiquer qu'un seul résultat est retourné
}

// Fonction Lua pour obtenir l'extension du fichier
int lua_getExtension(lua_State *L) {
    // Obtient le nombre d'arguments passés à la fonction Lua
    int argc = lua_gettop(L);
    // Vérifie qu'un seul argument est passé
    if (argc != 1) {
        return luaL_error(L, "Expected one argument"); // Retourne une erreur si ce n'est pas le cas
    }

    // Vérifie que le premier argument est une chaîne de caractères
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as the first argument");
    }

    // Récupère le chemin du fichier à partir de l'argument Lua
    const char *path = luaL_checkstring(L, 1);
    // Obtient l'extension du fichier à partir du chemin complet
    std::string extension = getExtension(path);
    // Pousse l'extension sur la pile Lua en tant que chaîne
    lua_pushstring(L, extension.c_str());
    return 1; // Retourne 1 pour indiquer qu'un seul résultat est retourné
}

// Fonction Lua pour obtenir le nom du fichier sans extension
int lua_getFilename(lua_State *L) {
    // Obtient le nombre d'arguments passés à la fonction Lua
    int argc = lua_gettop(L);
    // Vérifie qu'un seul argument est passé
    if (argc != 1) {
        return luaL_error(L, "Expected one argument"); // Retourne une erreur si ce n'est pas le cas
    }

    // Vérifie que le premier argument est une chaîne de caractères
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as the first argument");
    }

    // Récupère le chemin du fichier à partir de l'argument Lua
    const char *path = luaL_checkstring(L, 1);
    // Obtient le nom du fichier sans extension à partir du chemin complet
    std::string filename = getFilename(path);
    // Pousse le nom du fichier sur la pile Lua en tant que chaîne
    lua_pushstring(L, filename.c_str());
    return 1; // Retourne 1 pour indiquer qu'un seul résultat est retourné
}

// Fonction Lua pour obtenir le chemin parent
int lua_getPath(lua_State *L) {
    // Obtient le nombre d'arguments passés à la fonction Lua
    int argc = lua_gettop(L);
    // Vérifie qu'un seul argument est passé
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Vérifie que l'argument est une chaîne de caractères
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    // Récupère le chemin du fichier à partir de l'argument Lua
    const char *path = luaL_checkstring(L, 1);
    // Obtient le chemin parent à partir du chemin complet
    std::string p = getPath(path);
    // Pousse le chemin parent sur la pile Lua en tant que chaîne formatée
    lua_pushfstring(L, p.c_str());
    return 1;
}
