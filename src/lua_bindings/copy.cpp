#include "copy.hpp"
#include "fileOperations.hpp"
#include <string>

/**
 * @brief Lua-accessible function to copy a file
 *
 * This function is called from Lua and uses the copy_file function to copy a file.
 * It expects to receive two strings as arguments.
 * If the arguments are not strings, a Lua error is raised.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1: error message or nil)
 */
int lua_copy_file(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 2) {
        return luaL_error(L, "Expected two arguments");
    }

    if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        return luaL_error(L, "Expected two strings as arguments");
    }

    const char* src_path = luaL_checkstring(L, 1);
    const char* dest_path = luaL_checkstring(L, 2);

    std::string error_message = copy_file(src_path, dest_path);

    if (error_message.empty()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, error_message.c_str());
    }
    return 1;  // One return value (error message or nil)
}
