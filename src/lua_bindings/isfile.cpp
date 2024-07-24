#include "isfile.hpp"
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

namespace fs = std::filesystem;

/**
 * @brief Function to check if a path is a file
 *
 * This function takes a string_view representing the path and checks if it is a file.
 * It throws an exception if an error occurs.
 *
 * @param path The path to check
 * @return True if the path is a file, false otherwise
 * @throws std::invalid_argument if the path is empty
 * @throws std::system_error if an error occurs during the check
 */
bool is_file(const std::string_view& path) {
    if (path.empty()) {
        throw std::invalid_argument("Path cannot be empty");
    }

    std::error_code ec;
    bool result = fs::is_regular_file(path, ec);
    if (ec) {
        throw std::system_error(ec, "Error occurred while checking the file");
    }
    return result;
}

/**
 * @brief Lua-accessible function to check if a path is a file
 *
 * This function is called from Lua and uses the is_file function to check if a given path is a file.
 * It expects to receive a string as an argument.
 * If the argument is not a string, a Lua error is raised.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1 on success, the boolean result, or an error message on failure)
 */
int lua_isFile(lua_State* L) {
    // Ensure exactly one argument is passed
    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Ensure the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    // Get the path from the Lua stack
    size_t path_len;
    const char* path_data = lua_tolstring(L, 1, &path_len);
    std::string_view path(path_data, path_len);

    try {
        // Check if the path is a file
        bool result = is_file(path);

        // Push the result onto the Lua stack
        lua_pushboolean(L, result);
        return 1;
    } catch (const std::exception& e) {
        // In case of an error, push nil and the error message
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
    }
}
