#include "lua_bindings/moveTree.hpp"
#include <iostream>
#include <system_error>

namespace fs = std::filesystem;

/**
 * @brief Moves a directory tree from the source path to the destination path.
 *
 * This function moves a directory tree, including all its files and subdirectories,
 * from the source path to the destination path.
 *
 * @param source The source path of the directory tree to move.
 * @param destination The destination path where the directory tree will be moved.
 * @param error_message A string to hold the error message in case of failure.
 * @return True if the move was successful, false otherwise.
 */
bool moveTree(const fs::path& source, const fs::path& destination, std::string& error_message) {
    std::vector<std::tuple<fs::path, fs::path, fs::path>> symlinks;
    std::error_code ec;

    // Check if the source exists
    if (!fs::exists(source)) {
        error_message = "Source path does not exist: " + source.string();
        return false;
    }

    // Create the destination directory if it does not exist
    fs::create_directories(destination, ec);
    if (ec) {
        error_message = "Error creating destination directory: " + ec.message();
        return false;
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
                    error_message = "Error creating directory: " + dest_path.string() + " : " + ec.message();
                    return false;
                }
            } else if (fs::is_symlink(file_status)) {
                fs::path dest_link = fs::absolute(destination / path.lexically_relative(source));
                fs::path old_path = fs::read_symlink(path);
                fs::path new_path = fs::absolute(destination / fs::relative(old_path, source));
                symlinks.emplace_back(dest_link, old_path, new_path);
            } else {
                fs::rename(path, dest_path, ec);
                if (ec) {
                    error_message = "Error moving file: " + path.string() + " to " + dest_path.string() + " : " + ec.message();
                    return false;
                }
            }
        }
        // Remove the source directory tree after moving all files
        fs::remove_all(source, ec);
        if (ec) {
            error_message = "Error removing source directory: " + source.string() + " : " + ec.message();
            return false;
        }

        for (const auto& [dest_symlink, old_path, new_path] : symlinks) {
            if (fs::exists(new_path)) {
                fs::create_symlink(new_path, dest_symlink);
            } else if (fs::exists(old_path)) {
                fs::create_symlink(old_path, dest_symlink);
            }
        }
    } catch (const fs::filesystem_error& e) {
        error_message = "Filesystem error: " + std::string(e.what());
        return false;
    }

    return true;
}

/**
 * @brief Lua binding for moveTree function.
 *
 * This function wraps the moveTree function so it can be called from Lua.
 * It takes two arguments from the Lua stack:
 * 1. The source path of the directory tree to move.
 * 2. The destination path where the directory tree will be moved.
 *
 * It returns two values to Lua:
 * 1. A boolean indicating the success of the operation.
 * 2. A string containing the error message if the operation failed, or nil if it succeeded.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_moveTree(lua_State* L) {
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        return luaL_error(L, "Expected two strings as arguments");
    }

    const char* source = luaL_checkstring(L, 1);
    const char* destination = luaL_checkstring(L, 2);

    std::string error_message;
    bool result = moveTree(source, destination, error_message);

    lua_pushboolean(L, result);
    if (!result) {
        lua_pushstring(L, error_message.c_str());
    } else {
        lua_pushnil(L);
    }

    return 2;
}
