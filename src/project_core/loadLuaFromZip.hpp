#ifndef LOADLUAFROMZIP_HPP
#define LOADLUAFROMZIP_HPP

#include <string>
#include <lua.hpp>

/**
 * @brief Loads Lua scripts from a ZIP archive into the given Lua state.
 *
 * This function opens a ZIP archive, iterates through its entries, and loads any Lua scripts
 * found within into the provided Lua state.
 *
 * @param L The Lua state to load the scripts into.
 * @param zipname The path to the ZIP archive to be opened.
 */
void loadLuaFromZip(lua_State *L, const std::string &zipname);

#endif // LOADLUAFROMZIP_HPP

