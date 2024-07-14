#include "currentDir.hpp"
#include <filesystem>
#include <system_error>
#include <tuple>

/**
 * Get the current working directory.
 * @return A tuple containing the current directory path and an error message if any.
 */
std::tuple<std::string, std::string> currentDir() {
    std::error_code ec;
    auto path = std::filesystem::current_path(ec);
    if (ec) {
        return std::make_tuple("", "Error getting current directory: " + ec.message());
    }
    return std::make_tuple(path.string(), "");
}

/**
 * Lua binding for getting the current working directory.
 * @param L The Lua state.
 * @return Number of return values (2: current directory path and error message).
 * Lua usage: currentDir, err = lua_currentDir()
 */
int lua_currentDir(lua_State* L) {
    auto [path, error] = currentDir();

    lua_pushstring(L, path.c_str());
    if (error.empty()) {
        lua_pushnil(L); // No error, push nil
    } else {
        lua_pushstring(L, error.c_str());
    }

    return 2; // Two return values (current directory path and error message)
}

