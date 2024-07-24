#include "copy.hpp"
#include "fileOperations.hpp"
#include <filesystem>
#include <optional>
#include <string>

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
int lua_copy_file(lua_State* L) {
    // Check if there are two string arguments
    if (lua_gettop(L) != 2 || !lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        return luaL_error(L, "Expected two string arguments: source and destination paths");
    }

    // Retrieve the source and destination paths
    std::filesystem::path src_path(luaL_checkstring(L, 1));
    std::filesystem::path dest_path(luaL_checkstring(L, 2));

    // Copy file and push result
    auto result = custom_copy_file(src_path, dest_path);
    if (result) {
        lua_pushstring(L, result->c_str());
    } else {
        lua_pushnil(L);
    }

    return 1; // One return value (error message or nil)
}
