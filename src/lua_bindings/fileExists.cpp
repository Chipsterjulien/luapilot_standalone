#include "fileExists.hpp"
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

/**
 * @brief Check if a file exists and is a regular file.
 *
 * @param path The file path to check.
 * @return std::optional<std::string> An optional containing an error message if any.
 */
std::optional<std::string> fileExists(std::string_view path) {
    std::error_code ec;
    fs::path fsPath(path);

    // Combine existence and regular file checks into one using &&
    if (!fs::exists(fsPath, ec) || !fs::is_regular_file(fsPath, ec)) {
        if (ec) {
            return "Error checking file: " + ec.message();
        } else {
            return "Path exists but is not a regular file: " + fsPath.string();
        }
    }

    return std::nullopt; // File exists and is a regular file
}

/**
 * @brief Lua binding for checking if a file exists.
 *
 * @param L The Lua state.
 * @return int Number of return values (2: fileFound boolean and error message or nil).
 *
 * @note Lua usage: fileFound, err = lua_fileExists(path)
 *   - path: The file path to check.
 */
int lua_fileExists(lua_State* L) {
    // Check if there is one argument passed and it is a string
    if (lua_gettop(L) != 1 || !lua_isstring(L, 1)) {
        return luaL_error(L, "Expected one string argument");
    }

    // Get the file path from the Lua arguments
    std::string_view path = luaL_checkstring(L, 1);

    // Check if the file exists
    auto error_message = fileExists(path);

    // Push the result onto the Lua stack using the ternary operator for concise error handling
    lua_pushboolean(L, !error_message.has_value());
    lua_pushstring(L, error_message ? error_message->c_str() : nullptr);

    return 2; // Two return values (fileFound boolean and error message or nil)
}
