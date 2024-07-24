#include "copyTree.hpp"
#include <vector>
#include <tuple>
#include <system_error>
#include <iostream>

namespace fs = std::filesystem;

/**
 * @brief Utility function to copy a file and handle errors.
 *
 * @param source The source file path.
 * @param destination The destination file path.
 * @param continue_on_error Whether to continue on error or not.
 * @return std::optional<std::string> An optional string containing an error message if any, or an empty optional if successful.
 */
std::optional<std::string> copy_file_with_error_handling(const fs::path& source, const fs::path& destination, bool continue_on_error) {
    std::error_code ec;
    fs::copy_file(source, destination, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        if (continue_on_error) {
            std::cerr << "Warning: Error copying file: " << source.string() << " to " << destination.string() << " : " << ec.message() << '\n';
            return std::nullopt;
        } else {
            return "Error copying file: " + source.string() + " to " + destination.string() + " : " + ec.message();
        }
    }
    return std::nullopt;
}

/**
 * @brief Recursively copies a directory and its contents to a destination, handling symbolic links.
 *
 * This function copies the contents of the source directory to the destination directory.
 * It handles directories, regular files, and symbolic links appropriately.
 * Symbolic links are recreated at the destination, pointing to their respective targets.
 *
 * @param source The source directory to copy from.
 * @param destination The destination directory to copy to.
 * @param continue_on_error Whether to continue on error or not.
 * @return std::optional<std::string> An optional string containing an error message if any, or an empty optional if successful.
 */
std::optional<std::string> copy_directory(const fs::path& source, const fs::path& destination, bool continue_on_error) {
    std::vector<std::tuple<fs::path, fs::path, fs::path>> symlinks;
    std::error_code ec;
    bool has_warnings = false;

    // Check if the source directory exists and is valid
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
                    if (continue_on_error) {
                        std::cerr << "Warning: Error creating directory: " << dest_path.string() << " : " << ec.message() << '\n';
                        has_warnings = true;
                        continue; // Continue on error
                    } else {
                        return "Error creating directory: " + dest_path.string() + " : " + ec.message();
                    }
                }
            } else if (fs::is_regular_file(file_status)) {
                if (auto err = copy_file_with_error_handling(path, dest_path, continue_on_error); err) {
                    if (continue_on_error) {
                        std::cerr << "Warning: " << *err << '\n';
                        has_warnings = true;
                        continue; // Continue on error
                    } else {
                        return err;
                    }
                }
            } else if (fs::is_symlink(file_status)) {
                // Check for potential circular symlinks
                fs::path target = fs::read_symlink(path);
                fs::path full_target = fs::absolute(target); // Correct usage of fs::absolute
                if (full_target == fs::absolute(destination)) {
                    if (continue_on_error) {
                        std::cerr << "Warning: Circular symlink detected: " << path << " -> " << target << '\n';
                        has_warnings = true;
                        continue; // Skip circular symlinks
                    } else {
                        return "Circular symlink detected: " + path.string() + " -> " + target.string();
                    }
                }

                fs::path dest_link = fs::absolute(destination / path.lexically_relative(source));
                fs::path old_path = fs::absolute(fs::relative(path));
                symlinks.emplace_back(dest_link, old_path, fs::absolute(dest_path));
            } else {
                if (continue_on_error) {
                    std::cerr << "Warning: Unknown file type: " << path << '\n';
                    has_warnings = true;
                    continue; // Continue on error
                } else {
                    return "Unknown file type: " + path.string();
                }
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

    if (has_warnings) {
        return "Completed with warnings. Check the output for details.";
    }

    return std::nullopt;
}

/**
 * @brief Lua binding for the copy_directory function.
 *
 * This function is a Lua binding that exposes the copy_directory function to Lua scripts.
 * It expects two string arguments representing the source and destination directories.
 * The third optional argument indicates whether to continue on error.
 *
 * It returns one value to Lua:
 * A string containing the error message if the operation failed, or nil if it succeeded.
 *
 * @param L The Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_copyTree(lua_State* L) {
    // Check if there are at least two string arguments
    if (lua_gettop(L) < 2 || !lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        return luaL_error(L, "Expected at least two string arguments: source and destination paths");
    }

    // Retrieve the source and destination paths
    std::string_view src_path(luaL_checkstring(L, 1));
    std::string_view dest_path(luaL_checkstring(L, 2));

    // Check if the third argument is provided for continue_on_error
    bool continue_on_error = true; // Default to true
    if (lua_gettop(L) >= 3) {
        continue_on_error = lua_toboolean(L, 3);
    }

    // Copy directory and push result
    auto result = copy_directory(src_path, dest_path, continue_on_error);
    if (result) {
        lua_pushstring(L, result->c_str());
    } else {
        lua_pushnil(L);
    }

    return 1; // One return value (error message or nil)
}
