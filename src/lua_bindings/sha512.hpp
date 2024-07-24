#ifndef SHA512_HPP
#define SHA512_HPP

#include <openssl/evp.h>
#include <optional>
#include <string>
#include <lua.hpp>

/**
 * @brief Calculates the SHA-512 checksum of a file.
 *
 * @param path The path to the file.
 * @return An optional string containing the SHA-512 checksum, or nullopt if an error occurred.
 */
std::optional<std::string> sha512sum(const std::string &path);

/**
 * @brief Lua binding for the sha512sum function.
 *
 * @param L The Lua state.
 * @return 2 values: the SHA-512 checksum or nil, and an error message or nil.
 */
int lua_sha512sum(lua_State *L);

#endif // SHA512_HPP
