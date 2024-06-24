#include "lua_bindings/currentDir.hpp"
#include <filesystem>

/**
 * Get the current working directory.
 * @return A CurrentDirResult struct containing the current directory path and an error message if any.
 */
CurrentDirResult currentDir() {
    CurrentDirResult result = {"", ""};
    std::error_code ec;
    auto path = std::filesystem::current_path(ec);
    if (ec) {
        result.error_message = "Error getting current directory: " + ec.message();
    } else {
        result.path = path.string();
    }
    return result;
}

/**
 * Lua binding for getting the current working directory.
 * @param L The Lua state.
 * @return Number of return values (2: current directory path and error message).
 * Lua usage: path, error_message = lua_currentDir()
 */
int lua_currentDir(lua_State* L) {
    CurrentDirResult result = currentDir();

    lua_pushstring(L, result.path.c_str());
    lua_pushstring(L, result.error_message.c_str());

    return 2;
}