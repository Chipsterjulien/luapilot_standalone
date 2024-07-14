#include "listFiles.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

/**
 * Auxiliary function to list files.
 * @param L The Lua state.
 * @param basePath The base directory path.
 * @param path The current directory path.
 * @param index The index for the Lua table.
 * @param recursive Whether to list files recursively.
 */
void listFilesHelper(lua_State *L, const std::string& basePath, const std::string& path, int& index, bool recursive) {
    // Iterate over each entry in the given directory
    for (const auto &entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry)) { // Only process regular files
            std::string relativePath = fs::relative(entry.path(), basePath).string();

            lua_pushnumber(L, index++);           // Push the index onto the Lua stack
            lua_pushstring(L, relativePath.c_str());  // Push the modified file name onto the Lua stack
            lua_settable(L, -3);                  // Set the table entry
        }

        // If recursive and the entry is a directory, recursively call the function on that directory
        if (recursive && fs::is_directory(entry)) {
            listFilesHelper(L, basePath, entry.path().string(), index, recursive);
        }
    }
}

/**
 * Lua binding for listing files in a directory.
 * @param L The Lua state.
 * @return Number of return values (2: table of files and error message or nil).
 * Lua usage: files, err = lua_listFiles(path, recursive)
 *   - path: The directory path to list files from.
 *   - recursive (optional): Whether to list files recursively. Defaults to false.
 */
int lua_listFiles(lua_State *L) {
    // Check that at least one argument is passed
    int argc = lua_gettop(L);
    if (argc < 1) {
        return luaL_error(L, "Expected at least one argument");
    }

    const char *path = luaL_checkstring(L, 1); // Get the directory path from the first argument
    bool recursive = false; // Default value is false (non-recursive)

    // Check if the second argument is provided for recursion
    if (lua_gettop(L) >= 2) {
        recursive = lua_toboolean(L, 2); // Convert the second argument to a boolean
    }

    // Check if the directory exists
    if (!fs::exists(path)) {
        lua_pushnil(L);
        lua_pushstring(L, "Path does not exist");
        return 2; // Return nil and error message
    }

    // Check if the path is a directory
    if (!fs::is_directory(path)) {
        lua_pushnil(L);
        lua_pushstring(L, "Path is not a directory");
        return 2; // Return nil and error message
    }

    lua_newtable(L); // Create a new table on the Lua stack

    int index = 1; // Initialize the index for the Lua table
    // Call the auxiliary function to list the files
    listFilesHelper(L, path, path, index, recursive);

    lua_pushnil(L); // No error
    return 2; // Return the table of files and nil (no error)
}
