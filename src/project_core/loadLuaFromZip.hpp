#ifndef LOADLUAFROMZIP_HPP
#define LOADLUAFROMZIP_HPP

#include <string>
#include <lua.hpp>

void loadLuaFromZip(lua_State *L, const std::string &zipname);

#endif // LOADLUAFROMZIP_HPP
