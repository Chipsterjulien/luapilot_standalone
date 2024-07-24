#ifndef COPY_HPP
#define COPY_HPP

#include <lua.hpp>

/**
 * @brief Lua-accessible function to copy a file.
 *
 * This function is called from Lua and uses the custom_copy_file function to copy a file.
 * It expects to receive two strings as arguments.
 * If the arguments are not strings, a Lua error is raised.
 *
 * @param L Pointer to the Lua state.
 * @return Number of return values on the Lua stack (1: error message or nil).
 */
int lua_copy_file(lua_State* L);

#endif // COPY_HPP
