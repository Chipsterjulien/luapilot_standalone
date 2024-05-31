#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <lua.hpp>
#include <string>

// Lua wrapper for the getPath function
int lua_getBasename(lua_State *L);
int lua_getExtension(lua_State *L);
int lua_getFilename(lua_State *L);
int lua_getPath(lua_State *L);

#endif // FILE_UTILS_HPP