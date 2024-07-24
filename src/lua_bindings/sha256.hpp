#ifndef SHA256_HPP
#define SHA256_HPP

#include <openssl/evp.h>
#include <optional>
#include <string>
#include <lua.hpp>

/**
 * @brief Calculates the SHA-256 checksum of a file.
 *
 * @param path The path to the file.
 * @return An optional string containing the SHA-256 checksum, or nullopt if an error occurred.
 */
std::optional<std::string> sha256sum(const std::string &path);

/**
 * @brief Lua binding for the sha256sum function.
 *
 * @param L The Lua state.
 * @return 2 values: the SHA-256 checksum or nil, and an error message or nil.
 */
int lua_sha256sum(lua_State *L);

#endif // SHA256_HPP
