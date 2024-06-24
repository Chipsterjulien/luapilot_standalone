#include "lua_bindings/touch.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

/**
 * Function to create or update a file.
 * @param path The file path.
 * @return A TouchResult struct containing the success status and an error message if any.
 */
TouchResult touch(const std::string& path) {
    TouchResult result = {true, ""};

    try {
        // Check if the file exists
        if (fs::exists(path)) {
            // Update the last write time
            auto currentTime = fs::file_time_type::clock::now();
            fs::last_write_time(path, currentTime);
        } else {
            // Create an empty file
            std::ofstream file(path);
            if (!file) {
                result.success = false;
                result.error_message = "Failed to create the file: " + path;
            }
        }
    } catch (const fs::filesystem_error& e) {
        result.success = false;
        result.error_message = "Error: " + std::string(e.what());
    }

    return result;
}

/**
 * Lua binding for the touch function.
 * @param L The Lua state.
 * @return Number of return values (2: success boolean and error message).
 * Lua usage: success, error_message = lua_touch(path)
 *   - path: The file path to touch.
 */
int lua_touch(lua_State* L) {
    // Vérifier qu'il y a un argument passé
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Vérifier que l'argument est une chaîne de caractères
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    const char* path = luaL_checkstring(L, 1);

    TouchResult result = touch(path);

    lua_pushboolean(L, result.success);
    lua_pushstring(L, result.error_message.c_str());

    return 2;
}