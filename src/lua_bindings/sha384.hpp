#ifndef SHA384_HPP
#define SHA384_HPP

#include <lua.hpp>
#include <optional>
#include <string>

/**
 * @brief Computes the SHA-384 hash of a file.
 *
 * SHA-384 is part of the SHA-2 family (essentially a truncated SHA-512).
 * Useful for compatibility with specifications that mandate it
 * (TLS 1.3 cipher suites, Apple ecosystem, etc.).
 */
std::optional<std::string> sha384sum(const std::string &path);

int lua_sha384sum(lua_State *L);

#endif // SHA384_HPP
