#include "loadLuaFile.hpp"
#include <iostream>

/**
 * @brief Loads a Lua file into the given Lua state.
 *
 * This function reads the specified Lua file and loads its content into the provided Lua state.
 * If the file cannot be loaded, an error message is printed to the standard error output.
 *
 * @param L The Lua state to load the file into.
 * @param filename The path to the Lua file to be loaded.
 */
void loadLuaFile(lua_State *L, const std::string &filename) {
    if (luaL_dofile(L, filename.c_str()) != LUA_OK) {
        std::cerr << "Failed to load " << filename << ": " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1); // Remove error message from stack
    }
}
