#include "chdir.hpp"
#include <filesystem>

/**
 * Change the current working directory.
 * @param path The directory path to change to.
 * @return A ChdirResult struct containing the success status and an error message if any.
 */
ChdirResult chdir(const std::string& path) {
    ChdirResult result = {true, ""};
    std::error_code ec;
    std::filesystem::current_path(path, ec);
    if (ec) {
        result.success = false;
        result.error_message = "Error changing directory: " + ec.message();
    }
    return result;
}

/**
 * Lua binding for changing the current working directory.
 * @param L The Lua state.
 * @return Number of return values (2: success boolean and error message).
 * Lua usage: success, error_message = lua_chdir(path)
 *   - path: The directory path to change to.
 */
int lua_chdir(lua_State* L) {
    // Vérifier qu'il y a un argument passé
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Vérifier que l'argument est une chaîne de caractères
    if (lua_isnumber(L, 1)) {
        return luaL_error(L, "Expected a string, but got a number");
    }

    const char* path = luaL_checkstring(L, 1);

    ChdirResult result = chdir(path);

    lua_pushboolean(L, result.success);
    lua_pushstring(L, result.error_message.c_str());
    return 2;
}