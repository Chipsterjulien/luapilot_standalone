#ifndef COPYTREE_HPP
#define COPYTREE_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <lua.hpp>

/**
 * @brief Recursively copies a directory and its contents to a destination, handling symbolic links.
 *
 * This function copies the contents of the source directory to the destination directory.
 * It handles directories, regular files, and symbolic links appropriately.
 * Symbolic links are recreated at the destination, pointing to their respective targets.
 *
 * @param source The source directory to copy from.
 * @param destination The destination directory to copy to.
 * @param continue_on_error Whether to continue on error or not (default is true).
 * @return std::optional<std::string> An optional string containing an error message if any, or an empty optional if successful.
 */
std::optional<std::string> copy_directory(const std::filesystem::path& source, const std::filesystem::path& destination, bool continue_on_error = true);

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
int lua_copyTree(lua_State* L);

#endif // COPYTREE_HPP
