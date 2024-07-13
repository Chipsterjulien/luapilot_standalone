#ifndef SHA3_256_HPP
#define SHA3_256_HPP

#include <string>
#include <tuple>
#include <lua.hpp>

/**
 * @brief Calculates the SHA-3 256 checksum of a file.
 *
 * @param path The path to the file.
 * @return A tuple containing the SHA-3 256 checksum as a hexadecimal string and an error message if any.
 */
std::tuple<std::string, std::string> sha3_256sum(const std::string &path);

/**
 * @brief Lua binding for the sha3_256sum function.
 *
 * @param L The Lua state.
 * @return 2 values: the SHA-3 256 checksum or nil, and an error message or nil.
 */
int lua_sha3_256sum(lua_State *L);

#endif // SHA3_256_HPP
