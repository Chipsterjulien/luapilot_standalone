#include "lua_bindings/fileExists.hpp"
#include <filesystem>

/**
 * Check if a file exists and is a regular file.
 * @param path The file path to check.
 * @return A FileExistsResult struct containing the existence status and an error message if any.
 */
FileExistsResult fileExists(const std::string& path) {
    FileExistsResult result = {false, ""};
    std::error_code ec;
    if (std::filesystem::exists(path, ec)) {
        if (std::filesystem::is_regular_file(path, ec)) {
            result.exists = true;
        } else {
            result.error_message = "Path exists but is not a regular file: " + path;
        }
    } else if (ec) {
        result.error_message = "Error checking file: " + ec.message();
    }
    return result;
}

/**
 * Lua binding for checking if a file exists.
 * @param L The Lua state.
 * @return Number of return values (2: exists boolean and error message).
 * Lua usage: exists, error_message = lua_fileExists(path)
 *   - path: The file path to check.
 */
int lua_fileExists(lua_State* L) {
    // Vérifier qu'il y a un argument passé
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Vérifier que l'argument est une chaîne de caractères
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string");
    }

    // Récupérer le chemin du fichier à partir des arguments de lua
    const char* path = luaL_checkstring(L, 1);

    // Vérifier si le fichier existe
    FileExistsResult result = fileExists(path);

    // Pousser le résultat sur la pile lua
    lua_pushboolean(L, result.exists);
    lua_pushstring(L, result.error_message.c_str());

    return 2;
}
