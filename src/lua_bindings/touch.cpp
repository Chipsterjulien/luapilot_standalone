#include "touch.hpp"
#include <filesystem>
#include <fstream>
#include <system_error>
#include <cerrno>
#include <cstring> // Pour strerror

namespace fs = std::filesystem;

/**
 * Function to create or update a file.
 * @param path The file path.
 * @return A string containing an error message if any, or an empty string if successful.
 */
std::string touch(const std::string& path) {
    try {
        // Vérifiez si le répertoire parent a les permissions nécessaires
        fs::path parent_path = fs::path(path).parent_path();
        if (!parent_path.empty() && !fs::exists(parent_path)) {
            return "Parent directory does not exist: " + parent_path.string();
        }

        auto parent_status = fs::status(parent_path);
        if (!parent_path.empty() && (parent_status.permissions() & fs::perms::owner_write) == fs::perms::none) {
            return "No write permission in the parent directory: " + parent_path.string();
        }

        // Check if the file exists
        if (fs::exists(path)) {
            // Update the last write time
            auto currentTime = fs::file_time_type::clock::now();
            fs::last_write_time(path, currentTime);
        } else {
            // Create an empty file
            std::ofstream file(path);
            if (!file) {
                return "Failed to create the file: " + path + " (" + std::strerror(errno) + ")";
            }
        }
    } catch (const fs::filesystem_error& e) {
        return "Error: " + std::string(e.what());
    }

    return "";
}

/**
 * Lua binding for the touch function.
 * @param L The Lua state.
 * @return Number of return values (1: error message or nil).
 * Lua usage: error_message = lua_touch(path)
 *   - path: The file path to touch.
 */
int lua_touch(lua_State* L) {
    // Check that there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_argerror(L, 1, "Expected one argument: string path");
    }

    // Check that the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_argerror(L, 1, "Expected a string as argument");
    }

    const char* path = luaL_checkstring(L, 1);

    std::string error_message = touch(path);

    if (error_message.empty()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, error_message.c_str());
    }

    return 1;
}
