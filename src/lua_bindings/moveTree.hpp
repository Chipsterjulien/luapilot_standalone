#ifndef MOVETREE_HPP
#define MOVETREE_HPP

#include <lua.hpp>
#include <vector>
#include <filesystem>
#include <string>

/**
 * @brief Moves a directory tree from the source path to the destination path.
 *
 * This function moves a directory tree, including all its files and subdirectories,
 * from the source path to the destination path.
 *
 * @param source The source path of the directory tree to move.
 * @param destination The destination path where the directory tree will be moved.
 * @param error_message A string to hold the error message in case of failure.
 * @return True if the move was successful, false otherwise.
 */
bool moveTree(const std::filesystem::path& source, const std::filesystem::path& destination, std::string& error_message);

/**
 * @brief Lua binding for moveTree function.
 *
 * This function wraps the moveTree function so it can be called from Lua.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_moveTree(lua_State* L);

#endif // MOVETREE_HPP
