#ifndef LISTFILES_HPP
#define LISTFILES_HPP

#include <lua.hpp>

/**
 * @brief Lua binding for listing files in a directory.
 *
 * This function lists all files in a specified directory and returns the result as a Lua table.
 * It can list files recursively if the optional second argument is set to true.
 * It returns a Lua table of files and an error message or nil if there is no error.
 *
 * @param L The Lua state
 * @return Number of return values on the Lua stack (2: table of files and error message or nil)
 */
int lua_listFiles(lua_State *L);

#endif // LISTFILES_HPP
