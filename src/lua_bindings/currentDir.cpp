#include "currentDir.hpp"
#include <filesystem>
#include <system_error>

/**
 * Get the current working directory.
 * @return A string containing the current directory path if successful, or an error message if any.
 */
std::string currentDir() {
    std::error_code ec;
    auto path = std::filesystem::current_path(ec);
    if (ec) {
        return "Error getting current directory: " + ec.message();
    }
    return path.string();
}

/**
 * Lua binding for getting the current working directory.
 * @param L The Lua state.
 * @return Number of return values (1: current directory path or error message).
 * Lua usage: path_or_error = lua_currentDir()
 */
int lua_currentDir(lua_State* L) {
    std::string path_or_error = currentDir();

    lua_pushstring(L, path_or_error.c_str());

    return 1; // One return value (current directory path or error message)
}
