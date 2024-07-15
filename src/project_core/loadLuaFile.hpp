#ifndef LOADLUAFILE_HPP
#define LOADLUAFILE_HPP

#include <string>
#include <lua.hpp>

/**
 * @brief Loads a Lua file into the given Lua state.
 *
 * This function reads the specified Lua file and loads its content into the provided Lua state.
 *
 * @param L The Lua state to load the file into.
 * @param filename The path to the Lua file to be loaded.
 * @throws std::runtime_error if the file cannot be read or if there is a Lua error.
 */
void loadLuaFile(lua_State *L, const std::string &filename);

#endif // LOADLUAFILE_HPP
