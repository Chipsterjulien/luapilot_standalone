#ifndef LIST_FILES_HPP
#define LIST_FILES_HPP

#include <string>
#include <lua.hpp>

/**
 * Auxiliary function to list files.
 * @param L The Lua state.
 * @param basePath The base directory path.
 * @param path The current directory path.
 * @param index The index for the Lua table.
 * @param recursive Whether to list files recursively.
 */
void listFilesHelper(lua_State *L, const std::string &basePath, const std::string &path, int &index, bool recursive);

/**
 * Lua binding for listing files in a directory.
 * @param L The Lua state.
 * @return Number of return values (1: table of files).
 * Lua usage: files = lua_listFiles(path, recursive)
 *   - path: The directory path to list files from.
 *   - recursive (optional): Whether to list files recursively. Defaults to false.
 */
int lua_listFiles(lua_State *L);

#endif // LIST_FILES_HPP
