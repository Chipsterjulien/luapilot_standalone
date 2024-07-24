#ifndef CHECKSUM_UTILS_HPP
#define CHECKSUM_UTILS_HPP

#include <openssl/evp.h>
#include <optional>
#include <string>

/**
 * @brief Calculates the checksum of a file using the specified hash algorithm.
 *
 * @param path The path to the file.
 * @param md The EVP_MD object representing the hash algorithm.
 * @return An optional string containing the checksum, or an error message.
 */
std::optional<std::string> calculate_checksum(const std::string &path, const EVP_MD* md);

#endif // CHECKSUM_UTILS_HPP
