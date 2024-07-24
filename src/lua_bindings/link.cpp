#include "link.hpp"
#include <filesystem>
#include <string>
#include <system_error>
#include <lua.hpp>
#include <cstring>  // Pour strlen

namespace fs = std::filesystem;

/**
 * @brief Creates a symbolic link.
 *
 * This function takes two arguments:
 * 1. The target file or directory to link to.
 * 2. The path where the symbolic link will be created.
 *
 * If it fails, it returns the error message.
 * If successful, it returns an empty optional.
 *
 * @param target The target path for the symbolic link.
 * @param linkpath The path where the symbolic link will be created.
 * @return std::optional<std::string> An optional error message if an error occurs.
 */
std::optional<std::string> create_symlink(const std::string& target, const std::string& linkpath) {
    if (target.empty() || linkpath.empty()) {
        return "Target path and link path cannot be empty";
    }

    std::error_code ec;
    fs::path resolved_target = fs::weakly_canonical(fs::path(target), ec);
    if (ec) {
        return "Failed to resolve target path: " + ec.message();
    }

    try {
        if (fs::exists(linkpath)) {
            return "Link path already exists";
        }

        fs::create_directories(fs::path(linkpath).parent_path(), ec);
        if (ec) {
            throw fs::filesystem_error("Failed to create parent directories", ec);
        }

        fs::create_symlink(resolved_target, fs::path(linkpath), ec);
        if (ec) {
            throw fs::filesystem_error("Failed to create symbolic link", ec);
        }
    } catch (const std::exception& e) {
        return e.what();
    } catch (...) {
        return "Unknown error occurred";
    }

    return std::nullopt;
}

int lua_link(lua_State* L) {
    luaL_argcheck(L, lua_gettop(L) == 2, 1, "Expected two arguments");

    const char* target = luaL_checkstring(L, 1);
    const char* linkpath = luaL_checkstring(L, 2);

    std::optional<std::string> error = create_symlink(target, linkpath);
    if (error) {
        lua_pushnil(L);
        lua_pushstring(L, error->c_str());
        return 2;
    }

    lua_pushnil(L);
    return 1;
}
