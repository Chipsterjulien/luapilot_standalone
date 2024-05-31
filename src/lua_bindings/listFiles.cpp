#include "lua_bindings/listFiles.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void listFilesHelper(lua_State *L, const std::string& basePath, const std::string& path, int& index, bool recursive)
{
    for (const auto &entry : fs::directory_iterator(path))
    {
        if (fs::is_regular_file(entry)) // Only process regular files
        {
            std::string relativePath = fs::relative(entry.path(), basePath).string();

            if (relativePath.find("./") == 0)
            {
                relativePath = relativePath.substr(2); // Remove the "./" prefix
            }

            lua_pushnumber(L, index++);           // Push the index
            lua_pushstring(L, relativePath.c_str());  // Push the modified file name
            lua_settable(L, -3);                  // Set the table entry
        }

        if (recursive && fs::is_directory(entry))
        {
            listFilesHelper(L, basePath, entry.path().string(), index, recursive); // Recurse into subdirectory
        }
    }
}

int lua_listFiles(lua_State *L)
{
    const char *path = luaL_checkstring(L, 1);
    bool recursive = false; // Default value is false
    if (lua_gettop(L) >= 2) // Check if the second argument is provided
    {
        recursive = lua_toboolean(L, 2);
    }
    lua_newtable(L); // Create a new table on the Lua stack

    int index = 1;
    listFilesHelper(L, path, path, index, recursive);

    return 1; // Return the table
}
