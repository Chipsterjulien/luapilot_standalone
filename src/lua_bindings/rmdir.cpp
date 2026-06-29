#include "rmdir.hpp"
#include "lua_utils.hpp"
#include <filesystem>
#include <system_error>
#include <optional>
#include <string>
#include <string_view>
#include <lua.hpp>

namespace fs = std::filesystem;

/**
 * @brief Remove a file or an empty directory.
 *
 * Délègue directement à fs::remove et relaie son verdict. Pas de
 * pré-vérification (is_directory, permissions...) : ce serait à la fois
 * TOCTOU (le filesystem peut changer entre le check et l'action) et
 * incomplet (ignore les ACLs, le bit chattr +i, et le fait d'être root).
 * L'OS est seul juge.
 *
 * @param path The file or directory path to remove.
 * @return An optional string with the error message if any, or nullopt if successful.
 */
std::optional<std::string> rmdir(std::string_view path) {
    std::error_code ec;
    bool removed = fs::remove(fs::path(path), ec);
    if (ec) {
        return "cannot remove '" + std::string(path) + "': " + ec.message();
    }
    if (!removed) {
        // fs::remove renvoie false sans erreur quand le chemin n'existait pas.
        return "cannot remove '" + std::string(path) + "': no such file or directory";
    }
    return std::nullopt;
}

/**
 * @brief Remove a directory and all its contents (recursive).
 *
 * @param path The directory path to remove.
 * @return An optional string with the error message if any, or nullopt if successful.
 */
std::optional<std::string> rmdir_all(std::string_view path) {
    std::error_code ec;
    auto count = fs::remove_all(fs::path(path), ec);
    if (ec) {
        return "cannot remove '" + std::string(path) + "': " + ec.message();
    }
    if (count == 0) {
        // fs::remove_all renvoie 0 sans erreur quand le chemin n'existait pas.
        return "cannot remove '" + std::string(path) + "': no such file or directory";
    }
    return std::nullopt;
}

/**
 * @brief Lua binding for removing a file or an empty directory.
 *
 * Lua usage: ok, err = babet.rmdir(path)
 *
 * @param L The Lua state.
 * @return Number of return values on the Lua stack (2: ok/nil, err/nil).
 */
int lua_rmdir(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    const char *path = luaL_checkstring(L, 1);
    return push_action_result(L, rmdir(path));
}

/**
 * @brief Lua binding for removing a directory and all its contents.
 *
 * Lua usage: ok, err = babet.rmdirAll(path)
 *
 * @param L The Lua state.
 * @return Number of return values on the Lua stack (2: ok/nil, err/nil).
 */
int lua_rmdir_all(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    const char *path = luaL_checkstring(L, 1);
    return push_action_result(L, rmdir_all(path));
}
