#ifndef FILE_EXISTS_HPP
#define FILE_EXISTS_HPP

#include <lua.hpp>
#include <string>

// Lua wrapper for the fileExists function
int lua_fileExists(lua_State *L);

#endif // FILE_EXISTS_HPP
