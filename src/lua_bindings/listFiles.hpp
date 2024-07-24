#ifndef LISTFILES_HPP
#define LISTFILES_HPP

#include <lua.hpp>
#include <string>
#include <optional>
#include <filesystem>

namespace fs = std::filesystem; // Alias pour std::filesystem

/**
 * @brief Auxiliary function to list files.
 *
 * This function iterates over the directory and lists files.
 *
 * @param L The Lua state.
 * @param basePath The base directory path.
 * @param path The current directory path.
 * @param index The index for the Lua table.
 * @param recursive Whether to list files recursively.
 * @return std::optional<std::string> An error message if an error occurs, or std::nullopt if successful.
 */
std::optional<std::string> listFilesHelper(lua_State *L, const fs::path& basePath, const fs::path& path, int& index, bool recursive);

/**
 * @brief Lua binding for listing files in a directory.
 *
 * This function is accessible from Lua et permet de lister les fichiers dans un répertoire donné.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_listFiles(lua_State *L);

#endif // LISTFILES_HPP
