#ifndef BLAKE2B_HPP
#define BLAKE2B_HPP

#include <lua.hpp>
#include <optional>
#include <string>

/**
 * @brief Computes the BLAKE2b-512 hash of a file.
 *
 * BLAKE2b is a modern, fast cryptographic hash designed by the authors of
 * SHA-3 as a high-performance alternative to SHA-2. It is widely adopted
 * (WireGuard, Argon2, libsodium, modern Git proposals).
 *
 * @param path Path to the file to hash.
 * @return The 512-bit hash as a 128-character lowercase hex string,
 *         or std::nullopt if the file could not be read.
 */
std::optional<std::string> blake2b512sum(const std::string &path);

/**
 * @brief Lua binding: hash, err = luapilot.blake2b512sum(path)
 */
int lua_blake2b512sum(lua_State *L);

#endif // BLAKE2B_HPP
