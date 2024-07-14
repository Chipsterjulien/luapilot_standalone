#include "mkdir.hpp"
#include <filesystem>
#include <system_error>

/**
 * Create a directory path.
 * @param path The directory path to create.
 * @param ignore_if_exists If true, do not return an error if the directory already exists.
 * @return An error message if any, or an empty string if successful.
 */
std::string make_path(const std::string& path, bool ignore_if_exists) {
    try {
        if (!std::filesystem::create_directories(path)) {
            if (!ignore_if_exists && !std::filesystem::exists(path)) {
                return "Directory already exists or failed to create: " + path;
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        return "Error creating directory: " + std::string(e.what());
    }
    return "";
}

/**
 * Lua binding for creating a directory path.
 * @param L The Lua state.
 * @return Number of return values (1: error message or nil).
 * Lua usage: error_message = lua_mkdir(path, ignore_if_exists)
 *   - path: The directory path to create.
 *   - ignore_if_exists (optional): If true, do not return an error if the directory already exists. Defaults to false.
 */
int lua_mkdir(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc < 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check that the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    const char* path = luaL_checkstring(L, 1);
    bool ignore_if_exists = false;
    if (!lua_isnoneornil(L, 2)) {
        ignore_if_exists = lua_toboolean(L, 2);
    }

    std::string error_message = make_path(path, ignore_if_exists);

    if (error_message.empty()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, error_message.c_str());
    }
    return 1;
}
