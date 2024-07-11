#ifndef JOIN_HPP
#define JOIN_HPP

#include <lua.hpp>

/**
 * @brief Declaration of the lua_joinPath function for Lua
 *
 * This function is called from Lua to join multiple path segments.
 * It can take either a Lua table containing strings representing path segments
 * or multiple string arguments representing path segments.
 * It returns a single string representing the complete path.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1 on success, the joined path)
 */
int lua_joinPath(lua_State *L);

#endif // JOIN_HPP
