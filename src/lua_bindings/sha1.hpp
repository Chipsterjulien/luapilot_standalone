#ifndef SHA1_HPP
#define SHA1_HPP

#include <openssl/evp.h>
#include <optional>
#include <string>
#include <lua.hpp>

/**
 * @brief Calculates the SHA-1 checksum of a file.
 *
 * @param path The path to the file.
 * @return An optional string containing the SHA-1 checksum, or nullopt if an error occurred.
 */
std::optional<std::string> sha1sum(const std::string &path);

/**
 * @brief Lua binding for the sha1sum function.
 *
 * @param L The Lua state.
 * @return 2 values: the SHA-1 checksum or nil, and an error message or nil.
 */
int lua_sha1sum(lua_State *L);

#endif // SHA1_HPP
