#include "lua_bindings/copy.hpp"
#include "lua_bindings/fileOperations.hpp"
#include <string>

/**
 * @brief Lua-accessible function to copy a file
 *
 * This function is called from Lua and uses the copy_file function to copy a file.
 * It expects to receive two strings as arguments.
 * If the arguments are not strings, a Lua error is raised.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (2 on success or failure: boolean result and error message)
 */
int lua_copy_file(lua_State* L) {
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        return luaL_error(L, "Expected two strings as arguments");
    }

    const char* src_path = lua_tostring(L, 1);
    const char* dest_path = lua_tostring(L, 2);

    auto [success, error_message] = copy_file(src_path, dest_path);
    lua_pushboolean(L, success);
    lua_pushstring(L, error_message.c_str());
    return 2;  // Two return values (boolean result and error message)
}
