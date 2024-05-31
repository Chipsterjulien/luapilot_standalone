#ifndef LOADLUAFILE_HPP
#define LOADLUAFILE_HPP

#include <string>
#include <lua.hpp>

void loadLuaFile(lua_State *L, const std::string &filename);

#endif // LOADLUAFILE_HPP
