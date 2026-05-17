#ifndef LOADLUAFILE_HPP
#define LOADLUAFILE_HPP

#include <lua.hpp>
#include <string>

/**
 * @brief Loads and executes a Lua file.
 * @return true on success, false on error (error message printed to stderr).
 */
bool loadLuaFile(lua_State *L, const std::string &filename);

#endif // LOADLUAFILE_HPP