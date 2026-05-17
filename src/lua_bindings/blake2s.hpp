#ifndef BLAKE2S_HPP
#define BLAKE2S_HPP

#include <lua.hpp>
#include <optional>
#include <string>

/**
 * @brief Computes the BLAKE2s-256 hash of a file.
 *
 * BLAKE2s is the variant of BLAKE2 optimized for 32-bit platforms
 * (embedded systems). Sortie 256 bits.
 */
std::optional<std::string> blake2s256sum(const std::string &path);

int lua_blake2s256sum(lua_State *L);

#endif // BLAKE2S_HPP
