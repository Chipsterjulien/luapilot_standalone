#include "lua_bindings/fileSize.hpp"
#include <filesystem>

/**
 * Get the size of a file in bytes.
 * @param path The file path to check.
 * @return A FileSizeResult struct containing the size, success status, and an error message if any.
 */
FileSizeResult fileSize(const std::string& path) {
    FileSizeResult result = {false, 0, ""};
    std::error_code ec;
    if (std::filesystem::exists(path, ec) && std::filesystem::is_regular_file(path, ec)) {
        result.size = static_cast<std::uint64_t>(std::filesystem::file_size(path, ec));
        if (!ec) {
            result.success = true;
        } else {
            result.error_message = "Error getting file size: " + ec.message();
        }
    } else {
        result.error_message = "File does not exist or is not a regular file: " + path;
    }
    return result;
}

/**
 * Lua binding for getting the size of a file.
 * @param L The Lua state.
 * @return Number of return values (2: size and error message).
 * Lua usage: size, error_message = lua_fileSize(path)
 *   - path: The file path to check.
 */
int lua_fileSize(lua_State* L) {
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
    const char* path = luaL_checkstring(L, 1);

    // Obtenir la taille du fichier
    FileSizeResult result = fileSize(path);

    // Vérifier si l'obtention de la taille du fichier a échoué
    if (!result.success) {
        lua_pushnil(L);
        lua_pushstring(L, result.error_message.c_str());
        return 2;
    }

    // Pousser la taille du fichier sur la pile lua
    lua_pushinteger(L, static_cast<lua_Integer>(result.size));
    lua_pushstring(L, result.error_message.c_str());
    return 2;
}