#ifndef SHA3_384_HPP
#define SHA3_384_HPP

#include <lua.hpp>
#include <optional>
#include <string>

/**
 * @brief Computes the SHA3-384 hash of a file.
 *
 * Completes the SHA-3 family alongside sha3_256 and sha3_512 already
 * available in Babet.
 */
std::optional<std::string> sha3_384sum(const std::string &path);

int lua_sha3_384sum(lua_State *L);

#endif // SHA3_384_HPP
