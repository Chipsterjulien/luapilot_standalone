#ifndef SHA3_512_HPP
#define SHA3_512_HPP

#include <openssl/evp.h>
#include <optional>
#include <string>
#include <lua.hpp>

/**
 * @brief Calculates the SHA-3 512 checksum of a file.
 *
 * @param path The path to the file.
 * @return An optional string containing the SHA-3 512 checksum, or nullopt if an error occurred.
 */
std::optional<std::string> sha3_512sum(const std::string &path);

/**
 * @brief Lua binding for the sha3_512sum function.
 *
 * @param L The Lua state.
 * @return 2 values: the SHA-3 512 checksum or nil, and an error message or nil.
 */
int lua_sha3_512sum(lua_State *L);

#endif // SHA3_512_HPP
