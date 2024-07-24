#include "currentDir.hpp"
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Get the current working directory.
 *
 * @return std::optional<std::string> The current directory path if successful,
 *         or std::nullopt on failure.
 */
std::optional<std::string> currentDir() {
    std::error_code ec;
    auto path = fs::current_path(ec);
    if (ec) {
        return std::nullopt;
    }
    return path.string();
}

/**
 * @brief Lua binding for getting the current working directory.
 *
 * This function is a Lua binding that exposes the currentDir function to Lua scripts.
 * It returns the current directory path if successful, or an error message if it fails.
 *
 * @param L The Lua state.
 * @return int Number of return values (2: current directory path and error message).
 *
 * @note Lua usage: currentDir, err = lua_currentDir()
 */
int lua_currentDir(lua_State* L) {
    std::error_code ec;
    auto path = fs::current_path(ec);

    if (!ec) {
        lua_pushstring(L, path.string().c_str()); // Chemin du répertoire
        lua_pushnil(L);  // Pas d'erreur
    } else {
        lua_pushnil(L); // Pas de chemin
        lua_pushstring(L, ec.message().c_str()); // Message d'erreur plus détaillé
    }

    return 2;
}
