#ifndef FILEUTILS_HPP
#define FILEUTILS_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <lua.hpp>

/**
 * @brief Get the basename (filename with extension) from a full path.
 * @param fullPath The full path of the file.
 * @return An optional string with the basename, or an empty optional if an error occurs.
 */
std::optional<std::string> getBasename(std::string_view fullPath);

/**
 * @brief Get the extension of the file from a full path.
 * @param fullPath The full path of the file.
 * @return An optional string with the extension, or an empty optional if an error occurs.
 */
std::optional<std::string> getExtension(std::string_view fullPath);

/**
 * @brief Get the filename without extension from a full path.
 * @param fullPath The full path of the file.
 * @return An optional string with the filename without extension, or an empty optional if an error occurs.
 */
std::optional<std::string> getFilename(std::string_view fullPath);

/**
 * @brief Get the parent path from a full path.
 * @param fullPath The full path of the file.
 * @return An optional string with the parent path, or an empty optional if an error occurs.
 */
std::optional<std::string> getPath(std::string_view fullPath);

/**
 * @brief Lua binding for getting the basename.
 * @param L The Lua state.
 * @return Number of return values (2: basename, error).
 * Lua usage: basename, err = lua_getBasename(path)
 */
int lua_getBasename(lua_State* L);

/**
 * @brief Lua binding for getting the extension.
 * @param L The Lua state.
 * @return Number of return values (2: extension, error).
 * Lua usage: extension, err = lua_getExtension(path)
 */
int lua_getExtension(lua_State* L);

/**
 * @brief Lua binding for getting the filename without extension.
 * @param L The Lua state.
 * @return Number of return values (2: filename without extension, error).
 * Lua usage: filename, err = lua_getFilename(path)
 */
int lua_getFilename(lua_State* L);

/**
 * @brief Lua binding for getting the parent path.
 * @param L The Lua state.
 * @return Number of return values (2: parent path, error).
 * Lua usage: parentPath, err = lua_getPath(path)
 */
int lua_getPath(lua_State* L);

#endif // FILEUTILS_HPP
