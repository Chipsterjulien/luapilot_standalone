#include <filesystem>
#include <string>
#include <lua.hpp>

// Struct to hold the result of make_path function
struct MakePathResult {
    bool success;
    std::string error_message;
};

/**
 * Create a directory path.
 * @param path The directory path to create.
 * @param ignore_if_exists If true, do not return an error if the directory already exists.
 * @return A MakePathResult struct containing the success status and an error message if any.
 */
MakePathResult make_path(const std::string& path, bool ignore_if_exists) {
    MakePathResult result = {true, ""};
    try {
        if (!std::filesystem::create_directories(path)) {
            if (!ignore_if_exists) {
                result.success = false;
                result.error_message = "Directory already exists or failed to create: " + path;
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        result.success = false;
        result.error_message = "Error creating directory: " + std::string(e.what());
    }
    return result;
}

/**
 * Lua binding for creating a directory path.
 * @param L The Lua state.
 * @return Number of return values (2: success boolean and error message).
 * Lua usage: success, error_message = lua_mkdir(path, ignore_if_exists)
 *   - path: The directory path to create.
 *   - ignore_if_exists (optional): If true, do not return an error if the directory already exists. Defaults to false.
 */
int lua_mkdir(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool ignore_if_exists = false;
    if (!lua_isnoneornil(L, 2)) {
        ignore_if_exists = lua_toboolean(L, 2);
    }

    MakePathResult result = make_path(path, ignore_if_exists);

    lua_pushboolean(L, result.success);
    lua_pushstring(L, result.error_message.c_str());
    return 2;
}
