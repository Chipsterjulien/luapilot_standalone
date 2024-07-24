#include "moveTree.hpp"
#include <iostream>
#include <system_error>
#include <tuple>

namespace fs = std::filesystem;

/**
 * @brief Moves a directory tree from the source path to the destination path.
 *
 * This function moves a directory tree, including all its files and subdirectories,
 * from the source path to the destination path.
 *
 * @param source The source path of the directory tree to move.
 * @param destination The destination path where the directory tree will be moved.
 * @return An error message if any, or an empty string if successful.
 */
std::string moveTree(const fs::path& source, const fs::path& destination) {
    // Vector to store symlink mappings
    std::vector<std::tuple<fs::path, fs::path, fs::path>> symlink_mappings;
    std::error_code ec;

    // Check if the source exists and is a directory
    if (!fs::exists(source)) {
        return "Source path does not exist: " + source.string();
    }
    if (!fs::is_directory(source)) {
        return "Source path is not a directory: " + source.string();
    }

    // Create the destination directory if it does not exist
    fs::create_directories(destination, ec);
    if (ec) {
        return "Error creating destination directory: " + ec.message();
    }

    // Move each item in the source directory to the destination
    try {
        for (const auto& entry : fs::recursive_directory_iterator(source, fs::directory_options::skip_permission_denied)) {
            const auto& path = entry.path();
            fs::path relative_path = fs::relative(path, source);
            fs::path dest_path = destination / relative_path;

            auto file_status = fs::symlink_status(path);

            if (fs::is_directory(file_status) && !fs::is_symlink(file_status)) {
                fs::create_directories(dest_path, ec);
                if (ec) {
                    return "Error creating directory: " + dest_path.string() + " : " + ec.message();
                }
            } else if (fs::is_symlink(file_status)) {
                fs::path dest_symlink = fs::absolute(destination / path.lexically_relative(source));
                fs::path old_path = fs::read_symlink(path);
                fs::path new_path = fs::absolute(destination / fs::relative(old_path, source));
                symlink_mappings.emplace_back(dest_symlink, old_path, new_path);
            } else {
                fs::rename(path, dest_path, ec);
                if (ec) {
                    return "Error moving file: " + path.string() + " to " + dest_path.string() + " : " + ec.message();
                }
            }
        }

        // Remove the source directory tree after moving all files
        fs::remove_all(source, ec);
        if (ec) {
            return "Error removing source directory: " + source.string() + " : " + ec.message();
        }

        // Recreate symlinks in the destination
        for (const auto& [dest_symlink, old_path, new_path] : symlink_mappings) {
            if (fs::exists(new_path)) {
                fs::create_symlink(new_path, dest_symlink);
            } else if (fs::exists(old_path)) {
                fs::create_symlink(old_path, dest_symlink);
            }
        }
    } catch (const fs::filesystem_error& e) {
        return "Filesystem error: " + std::string(e.what());
    }

    return "";
}

/**
 * @brief Lua binding for moveTree function.
 *
 * This function wraps the moveTree function so it can be called from Lua.
 * It takes two arguments from the Lua stack:
 * 1. The source path of the directory tree to move.
 * 2. The destination path where the directory tree will be moved.
 *
 * It returns one value to Lua:
 * 1. A string containing the error message if the operation failed, or nil if it succeeded.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_moveTree(lua_State* L) {
    // Check if there are exactly two arguments
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "Expected two arguments");
    }

    // Check if both arguments are strings
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        return luaL_error(L, "Expected two strings as arguments");
    }

    const char* source = luaL_checkstring(L, 1);
    const char* destination = luaL_checkstring(L, 2);

    std::string error_message = moveTree(source, destination);

    if (error_message.empty()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, error_message.c_str());
    }

    return 1;
}
