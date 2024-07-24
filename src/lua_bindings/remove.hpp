#ifndef REMOVE_HPP
#define REMOVE_HPP

#include <filesystem>
#include <string>
#include <system_error>
#include <lua.hpp>

/**
 * @brief Function to remove a file
 *
 * This function takes a string representing the path of the file,
 * and removes the file.
 *
 * @param path The path of the file to remove
 * @return An error message if any, or an empty string if successful.
 */
std::string remove_file(const std::string& path);

/**
 * @brief Lua-accessible function to remove a file
 *
 * This function is called from Lua and uses the remove_file function to remove a file.
 * It expects to receive a string as an argument.
 * If the argument is not a string, a Lua error is raised.
 *
 * @param L Pointer to the Lua state
 * @return Number of return values on the Lua stack (1: error message or nil).
 */
int lua_remove_file(lua_State* L);

#endif // REMOVE_HPP
