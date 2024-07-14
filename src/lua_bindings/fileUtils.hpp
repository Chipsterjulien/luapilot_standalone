#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <string>
#include <tuple>
#include <system_error>
#include <lua.hpp>

/**
 * Get the basename (filename with extension) from a full path.
 * @param fullPath The full path of the file.
 * @return A tuple with the basename and an error code if any.
 */
std::tuple<std::string, std::error_code> getBasename(const std::string &fullPath);

/**
 * Get the extension of the file from a full path.
 * @param fullPath The full path of the file.
 * @return A tuple with the extension and an error code if any.
 */
std::tuple<std::string, std::error_code> getExtension(const std::string &fullPath);

/**
 * Get the filename without extension from a full path.
 * @param fullPath The full path of the file.
 * @return A tuple with the filename without extension and an error code if any.
 */
std::tuple<std::string, std::error_code> getFilename(const std::string &fullPath);

/**
 * Get the parent path from a full path.
 * @param fullPath The full path of the file.
 * @return A tuple with the parent path and an error code if any.
 */
std::tuple<std::string, std::error_code> getPath(const std::string &fullPath);

/**
 * Lua binding for getting the basename.
 * @param L The Lua state.
 * @return Number of return values (2: basename, error).
 * Lua usage: basename, err = lua_getBasename(path)
 */
int lua_getBasename(lua_State *L);

/**
 * Lua binding for getting the extension.
 * @param L The Lua state.
 * @return Number of return values (2: extension, error).
 * Lua usage: extension, err = lua_getExtension(path)
 */

/**
 * Lua binding for getting the extension.
 * @param L The Lua state.
 * @return Number of return values (2: extension, error).
 * Lua usage: extension, err = lua_getExtension(path)
 */
int lua_getExtension(lua_State *L);

/**
 * Lua binding for getting the filename without extension.
 * @param L The Lua state.
 * @return Number of return values (2: filename without extension, error).
 * Lua usage: filename, err = lua_getFilename(path)
 */
int lua_getFilename(lua_State *L);

/**
 * Lua binding for getting the parent path.
 * @param L The Lua state.
 * @return Number of return values (2: parent path, error).
 * Lua usage: parentPath, err = lua_getPath(path)
 */
int lua_getPath(lua_State *L);

#endif // FILE_UTILS_HPP
