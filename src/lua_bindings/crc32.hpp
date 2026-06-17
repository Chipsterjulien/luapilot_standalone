#ifndef CRC32_HPP
#define CRC32_HPP

#include <lua.hpp>
#include <optional>
#include <string>

/**
 * @brief Compute CRC32 of a memory buffer.
 * @param data Pointer to the data bytes.
 * @param len Number of bytes.
 * @return CRC32 as a lowercase 8-character hex string.
 *
 * Cannot fail (pure computation on memory).
 */
std::string crc32_hex(const unsigned char *data, size_t len);

/**
 * @brief Compute CRC32 of a file, streaming (no full-file load in RAM).
 * @param path Filesystem path.
 * @param err_out Filled with an error message if std::nullopt is returned.
 * @return CRC32 as a lowercase 8-character hex string, or std::nullopt
 *         on failure.
 *
 * Refuses directories and non-regular files explicitly.
 */
std::optional<std::string> crc32sum(const std::string &path,
                                    std::string &err_out);

/**
 * @brief Lua binding: luapilot.crc32(data) -> string
 *        Returns the CRC32 hex of an in-memory binary-safe string.
 *        Cannot fail at runtime; raises via luaL_error only on
 *        argument-type bugs.
 */
int lua_crc32(lua_State *L);

/**
 * @brief Lua binding: luapilot.crc32sum(path) -> (string, nil) | (nil, err)
 *        Returns the CRC32 hex of a file's content, streaming.
 *        Matches the contract of md5sum / sha256sum etc.
 */
int lua_crc32sum(lua_State *L);

#endif // CRC32_HPP
