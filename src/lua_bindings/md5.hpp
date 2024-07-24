#ifndef MD5_HPP
#define MD5_HPP

#include <lua.hpp>
#include <string>
#include <tuple>

/**
 * @brief Calculates the MD5 checksum of a file.
 *
 * @param path The path to the file.
 * @return A tuple containing the MD5 checksum as a hexadecimal string and an error message if any.
 */
std::tuple<std::string, std::string> md5sum(const std::string& path);

/**
 * @brief Lua binding for the md5sum function.
 *
 * @param L The Lua state.
 * @return 2 values: the MD5 checksum or nil, and an error message or nil.
 */
int lua_md5sum(lua_State* L);

#endif // MD5_HPP
