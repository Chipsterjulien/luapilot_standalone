#include "copyTree.hpp"
#include <iostream>
#include <system_error>

namespace fs = std::filesystem;

/**
 * @brief Recursively copies a directory and its contents to a destination, handling symbolic links.
 *
 * This function copies the contents of the source directory to the destination directory.
 * It handles directories, regular files, and symbolic links appropriately.
 * Symbolic links are recreated at the destination, pointing to their respective targets.
 *
 * @param source The source directory to copy from.
 * @param destination The destination directory to copy to.
 * @return A string with the error message if any, or an empty string if successful.
 */
std::string copy_directory(const fs::path& source, const fs::path& destination) {
    std::vector<std::tuple<fs::path, fs::path, fs::path>> symlinks;
    std::error_code ec;

    if (!fs::exists(source)) {
        return "Source directory does not exist: " + source.string();
    }
    if (!fs::is_directory(source)) {
        return "Source is not a directory: " + source.string();
    }

    // Create the destination directory if it does not exist
    fs::create_directories(destination, ec);
    if (ec) {
        return "Error creating destination directory: " + ec.message();
    }

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
            } else if (fs::is_regular_file(file_status)) {
                fs::copy(path, dest_path, fs::copy_options::overwrite_existing, ec);
                if (ec) {
                    return "Error copying file: " + path.string() + " to " + dest_path.string() + " : " + ec.message();
                }
            } else if (fs::is_symlink(file_status)) {
                fs::path dest_link = fs::absolute(destination / path.lexically_relative(source));
                fs::path old_path = fs::absolute(fs::relative(path));
                symlinks.emplace_back(dest_link, old_path, fs::absolute(dest_path));
            } else {
                std::cerr << "Unknown file type: " << path << '\n';
            }
        }

        for (const auto& [dest_symlink, old_path, new_path] : symlinks) {
            if (fs::exists(new_path)) {
                fs::create_symlink(new_path, dest_symlink);
            } else {
                fs::create_symlink(old_path, dest_symlink);
            }
        }
    } catch (const fs::filesystem_error& e) {
        return "Error copying directory: " + std::string(e.what());
    }
    return "";
}

/**
 * @brief Lua binding for the copy_directory function.
 *
 * This function is a Lua binding that exposes the copy_directory function to Lua scripts.
 * It expects two string arguments representing the source and destination directories.
 *
 * It returns one value to Lua:
 * A string containing the error message if the operation failed, or nil if it succeeded.
 *
 * @param L The Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_copyTree(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "Expected two arguments");
    }

    if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        return luaL_error(L, "Expected two strings as arguments");
    }

    const char* source = luaL_checkstring(L, 1);
    const char* destination = luaL_checkstring(L, 2);

    std::string error_message = copy_directory(source, destination);

    if (error_message.empty()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, error_message.c_str());
    }

    return 1;  // One return value (error message or nil)
}
