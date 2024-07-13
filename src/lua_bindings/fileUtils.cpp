#include "fileUtils.hpp"
#include <filesystem>

namespace fs = std::filesystem;

/**
 * Get the basename (filename with extension) from a full path.
 * @param fullPath The full path of the file.
 * @return The basename as a string.
 */
std::string getBasename(const std::string& fullPath) {
    return fs::path(fullPath).filename().string();
}

/**
 * Get the extension of the file from a full path.
 * @param fullPath The full path of the file.
 * @return The extension as a string.
 */
std::string getExtension(const std::string& fullPath) {
    return fs::path(fullPath).extension().string();
}

/**
 * Get the filename without extension from a full path.
 * @param fullPath The full path of the file.
 * @return The filename without extension as a string.
 */
std::string getFilename(const std::string& fullPath) {
    return fs::path(fullPath).stem().string();
}

/**
 * Get the parent path from a full path.
 * @param fullPath The full path of the file.
 * @return The parent path as a string.
 */
std::string getPath(const std::string& fullPath) {
    return fs::path(fullPath).parent_path().string();
}

/**
 * Lua binding for getting the basename.
 * @param L The Lua state.
 * @return Number of return values (1: basename).
 * Lua usage: basename = lua_getBasename(path)
 */
int lua_getBasename(lua_State* L) {
    // Vérifier qu'il y a un argument passé
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Vérifier que le premier argument est une chaîne de caractères
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as the first argument");
    }

    // Récupérer le chemin du fichier à partir de l'argument Lua
    const char* path = luaL_checkstring(L, 1);
    // Obtenir le nom de base à partir du chemin complet
    std::string basename = getBasename(path);
    // Pousser le nom de base sur la pile Lua en tant que chaîne
    lua_pushstring(L, basename.c_str());
    return 1; // Retourne 1 pour indiquer qu'un seul résultat est retourné
}

/**
 * Lua binding for getting the extension.
 * @param L The Lua state.
 * @return Number of return values (1: extension).
 * Lua usage: extension = lua_getExtension(path)
 */
int lua_getExtension(lua_State* L) {
    // Vérifier qu'il y a un argument passé
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Vérifier que le premier argument est une chaîne de caractères
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as the first argument");
    }

    // Récupérer le chemin du fichier à partir de l'argument Lua
    const char* path = luaL_checkstring(L, 1);
    // Obtenir l'extension du fichier à partir du chemin complet
    std::string extension = getExtension(path);
    // Pousser l'extension sur la pile Lua en tant que chaîne
    lua_pushstring(L, extension.c_str());
    return 1; // Retourne 1 pour indiquer qu'un seul résultat est retourné
}

/**
 * Lua binding for getting the filename without extension.
 * @param L The Lua state.
 * @return Number of return values (1: filename without extension).
 * Lua usage: filename = lua_getFilename(path)
 */
int lua_getFilename(lua_State* L) {
    // Vérifier qu'il y a un argument passé
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Vérifier que le premier argument est une chaîne de caractères
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as the first argument");
    }

    // Récupérer le chemin du fichier à partir de l'argument Lua
    const char* path = luaL_checkstring(L, 1);
    // Obtenir le nom du fichier sans extension à partir du chemin complet
    std::string filename = getFilename(path);
    // Pousser le nom du fichier sur la pile Lua en tant que chaîne
    lua_pushstring(L, filename.c_str());
    return 1; // Retourne 1 pour indiquer qu'un seul résultat est retourné
}

/**
 * Lua binding for getting the parent path.
 * @param L The Lua state.
 * @return Number of return values (1: parent path).
 * Lua usage: parentPath = lua_getPath(path)
 */
int lua_getPath(lua_State* L) {
    // Vérifier qu'il y a un argument passé
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Vérifier que l'argument est une chaîne de caractères
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    // Récupérer le chemin du fichier à partir de l'argument Lua
    const char* path = luaL_checkstring(L, 1);
    // Obtenir le chemin parent à partir du chemin complet
    std::string p = getPath(path);
    // Pousser le chemin parent sur la pile Lua en tant que chaîne formatée
    lua_pushstring(L, p.c_str());
    return 1; // Retourne 1 pour indiquer qu'un seul résultat est retourné
}