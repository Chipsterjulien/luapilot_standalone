#ifndef COPYTREE_HPP
#define COPYTREE_HPP

#include <lua.hpp>
#include <filesystem>
#include <string>
#include <vector>

/**
 * @brief Copy a source directory to a destination
 *
 * This function recursively copies all files and subdirectories
 * from the source directory to the destination directory.
 *
 * @param source The path of the source directory
 * @param destination The path of the destination directory
 * @return A string with the error message if any, or an empty string if successful.
 */
std::string copy_directory(const std::filesystem::path& source, const std::filesystem::path& destination);

/**
 * @brief Lua-accessible function to copy a directory
 *
 * This function is called from Lua and uses the copy_directory
 * function to copy a directory. It expects to receive two strings
 * as arguments: the source path and the destination path.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1: error message or nil)
 */
int lua_copyTree(lua_State* L);

#endif // COPYTREE_HPP
