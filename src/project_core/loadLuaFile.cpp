#include "loadLuaFile.hpp"
#include <iostream>

void loadLuaFile(lua_State *L, const std::string &filename) {
    if (luaL_dofile(L, filename.c_str()) != LUA_OK) {
        std::cerr << "Failed to load " << filename << ": " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1); // Remove error message from stack
    }
}
