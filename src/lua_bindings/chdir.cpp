#include "chdir.hpp"
#include <filesystem>
#include <string>
#include <system_error>
#include <optional>
#include <lua.hpp>
#include <cstring>

namespace fs = std::filesystem;

/**
 * @brief Change the current working directory.
 *
 * @param path The directory path to change to.
 * @return std::optional<std::string> An optional string containing an error message if any, or an empty optional if successful.
 */
std::optional<std::string> chdir(const fs::path& path) {
    std::error_code ec;

    // Convert relative path to absolute path and then to canonical path
    fs::path absolute_path = fs::absolute(path, ec);
    if (ec) {
        return "Error resolving absolute path for '" + path.string() + "': " + ec.message();
    }

    fs::path canonical_path = fs::canonical(absolute_path, ec);
    if (ec) {
        return "Error resolving canonical path for '" + absolute_path.string() + "': " + ec.message();
    }

    // Check if the path is a directory
    if (!fs::is_directory(canonical_path, ec)) {
        return "Path '" + canonical_path.string() + "' is not a directory or cannot be accessed: " + ec.message();
    }

    // Change current working directory
    fs::current_path(canonical_path, ec);
    if (ec) {
        return "Error changing directory to '" + canonical_path.string() + "': " + ec.message();
    }
    return std::nullopt;
}

/**
 * @brief Lua binding for changing the current working directory.
 *
 * @param L The Lua state.
 * @return int Number of return values (1: error message or nil).
 * @note Lua usage: error_message = lua_chdir(path)
 *   - path: The directory path to change to.
 */
int lua_chdir(lua_State* L) {
    // Check if there is one string argument
    size_t len;
    const char* path_cstr = luaL_checklstring(L, 1, &len);
    if (!path_cstr) {
        return luaL_error(L, "Expected one string argument");
    }

    fs::path path(std::string_view(path_cstr, len));

    // Change directory
    auto result = chdir(path);
    if (result) {
        lua_pushnil(L);
        lua_pushlstring(L, result->c_str(), result->size()); // Error, push error message
        return 2;
    } else {
        lua_pushnil(L); // Success, push nil
        return 1;
    }
}
