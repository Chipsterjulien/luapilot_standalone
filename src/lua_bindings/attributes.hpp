#ifndef ATTRIBUTES_HPP
#define ATTRIBUTES_HPP

#include <string>
#include <optional>
#include <filesystem>
#include <lua.hpp>
#include "lua_utils.hpp"

/**
 * @brief Changes the owner, group, and permissions of a file or directory.
 *
 * @param path The directory path to change to.
 * @param owner The new owner (UID).
 * @param group The new group (GID).
 * @param mode The new permissions (mode).
 * @return std::optional<std::string> An optional string containing an error message if any, or an empty optional if successful.
 */
int lua_setattr(lua_State* L);

/**
 * @brief Gets the attributes of a file or directory.
 *
 * @param L The Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_getattr(lua_State* L);

#endif // ATTRIBUTES_HPP
