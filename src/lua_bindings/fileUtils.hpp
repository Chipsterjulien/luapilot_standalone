#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <string>
#include <lua.hpp>

/**
 * Get the basename (filename with extension) from a full path.
 * @param fullPath The full path of the file.
 * @return The basename as a string.
 */
std::string getBasename(const std::string &fullPath);

/**
 * Get the extension of the file from a full path.
 * @param fullPath The full path of the file.
 * @return The extension as a string.
 */
std::string getExtension(const std::string &fullPath);

/**
 * Get the filename without extension from a full path.
 * @param fullPath The full path of the file.
 * @return The filename without extension as a string.
 */
std::string getFilename(const std::string &fullPath);

/**
 * Get the parent path from a full path.
 * @param fullPath The full path of the file.
 * @return The parent path as a string.
 */
std::string getPath(const std::string &fullPath);

/**
 * Lua binding for getting the basename.
 * @param L The Lua state.
 * @return Number of return values (1: basename).
 * Lua usage: basename = lua_getBasename(path)
 */
int lua_getBasename(lua_State *L);

/**
 * Lua binding for getting the extension.
 * @param L The Lua state.
 * @return Number of return values (1: extension).
 * Lua usage: extension = lua_getExtension(path)
 */
int lua_getExtension(lua_State *L);

/**
 * Lua binding for getting the filename without extension.
 * @param L The Lua state.
 * @return Number of return values (1: filename without extension).
 * Lua usage: filename = lua_getFilename(path)
 */
int lua_getFilename(lua_State *L);

/**
 * Lua binding for getting the parent path.
 * @param L The Lua state.
 * @return Number of return values (1: parent path).
 * Lua usage: parentPath = lua_getPath(path)
 */
int lua_getPath(lua_State *L);

#endif // FILE_UTILS_HPP
