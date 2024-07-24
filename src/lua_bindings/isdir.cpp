#include "isdir.hpp"
#include <filesystem>
#include <string>
#include <system_error>
#include <stdexcept>

namespace fs = std::filesystem;

/**
 * @brief Function to check if a path is a directory
 *
 * This function takes a string representing the path and checks if it is a directory.
 * It throws an exception if an error occurs.
 *
 * @param path The path to check
 * @return True if the path is a directory, false otherwise
 * @throws std::system_error if an error occurs during the check
 */
bool is_directory(const std::string& path) {
    if (path.empty()) {
        throw std::invalid_argument("Path cannot be empty");
    }

    std::error_code ec;
    bool result = fs::is_directory(fs::path(path), ec);
    if (ec) {
        throw std::system_error(ec, "Error occurred while checking the directory");
    }
    return result;
}

/**
 * @brief Lua-accessible function to check if a path is a directory
 *
 * This function is called from Lua and uses the is_directory function to check if a given path is a directory.
 * It expects to receive a string as an argument.
 * If the argument is not a string, a Lua error is raised.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1 on success, the boolean result, or an error message on failure)
 */
int lua_isDir(lua_State* L) {
    // Ensure exactly one argument is passed
    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Ensure the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    // Get the path from the Lua stack
    std::string path = lua_tostring(L, 1);

    try {
        // Check if the path is a directory
        bool result = is_directory(path);

        // Push the result onto the Lua stack
        lua_pushboolean(L, result);
        return 1;
    } catch (const std::exception& e) {
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
    }
}
