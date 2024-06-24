#include "lua_bindings/listFiles.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

/**
 * Auxiliary function to list files.
 * @param L The Lua state.
 * @param basePath The base directory path.
 * @param path The current directory path.
 * @param index The index for the Lua table.
 * @param recursive Whether to list files recursively.
 */
void listFilesHelper(lua_State *L, const std::string& basePath, const std::string& path, int& index, bool recursive) {
    // Itère sur chaque entrée dans le répertoire donné
    for (const auto &entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry)) { // Ne traite que les fichiers réguliers
            std::string relativePath = fs::relative(entry.path(), basePath).string();

            // Supprime le préfixe "./" du chemin relatif s'il est présent
            if (relativePath.find("./") == 0) {
                relativePath = relativePath.substr(2);
            }

            lua_pushnumber(L, index++);           // Pousse l'index sur la pile Lua
            lua_pushstring(L, relativePath.c_str());  // Pousse le nom de fichier modifié sur la pile Lua
            lua_settable(L, -3);                  // Définit l'entrée de la table
        }

        // Si récursif et que l'entrée est un répertoire, appelle récursivement la fonction sur ce répertoire
        if (recursive && fs::is_directory(entry)) {
            listFilesHelper(L, basePath, entry.path().string(), index, recursive);
        }
    }
}

/**
 * Lua binding for listing files in a directory.
 * @param L The Lua state.
 * @return Number of return values (1: table of files).
 * Lua usage: files = lua_listFiles(path, recursive)
 *   - path: The directory path to list files from.
 *   - recursive (optional): Whether to list files recursively. Defaults to false.
 */
int lua_listFiles(lua_State *L) {
    const char *path = luaL_checkstring(L, 1); // Récupère le chemin du répertoire à partir du premier argument
    bool recursive = false; // Valeur par défaut est false (non récursif)

    // Vérifie si le second argument est fourni pour la récursivité
    if (lua_gettop(L) >= 2) {
        recursive = lua_toboolean(L, 2); // Convertit le second argument en booléen
    }

    lua_newtable(L); // Crée une nouvelle table sur la pile Lua

    int index = 1; // Initialise l'index pour la table Lua
    // Appelle la fonction auxiliaire pour lister les fichiers
    listFilesHelper(L, path, path, index, recursive);

    return 1;
}