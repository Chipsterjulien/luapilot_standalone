#include "lua_bindings/copyTree.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <lua.hpp>

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
 * @return true if the directory was copied successfully, false otherwise.
 */
bool copy_directory(const fs::path& source, const fs::path& destination) {
    std::vector<std::tuple<fs::path, fs::path, fs::path>> symlinks;
    try {
        if (!fs::exists(source) || !fs::is_directory(source)) {
            std::cerr << "Source directory does not exist or is not a directory\n";
            return false;
        }

        fs::create_directories(destination);

        for (const auto& entry : fs::recursive_directory_iterator(source, fs::directory_options::skip_permission_denied)) {
            const auto& path = entry.path();
            auto relative_path = fs::relative(path, source);
            fs::path dest_path = destination / relative_path;

            auto file_status = fs::symlink_status(path);

            if (fs::is_directory(file_status) && !fs::is_symlink(file_status)) {
                fs::create_directories(dest_path);
            } else if (fs::is_regular_file(file_status)) {
                fs::copy(path, dest_path, fs::copy_options::overwrite_existing);
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
    } catch (fs::filesystem_error& e) {
        std::cerr << "Erreur lors de la copie du répertoire : " << e.what() << '\n';
        return false;
    }
    return true;
}

/**
 * @brief Lua binding for the copy_directory function.
 *
 * This function is a Lua binding that exposes the copy_directory function to Lua scripts.
 * It expects two string arguments representing the source and destination directories.
 *
 * @param L The Lua state.
 * @return The number of return values (1 boolean value indicating success or failure).
 */
int lua_copyTree(lua_State* L) {
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        return luaL_error(L, "Expected two strings as arguments");
    }

    const char* source = lua_tostring(L, 1);
    const char* destination = lua_tostring(L, 2);

    bool result = copy_directory(source, destination);
    lua_pushboolean(L, result);
    return 1;
}
