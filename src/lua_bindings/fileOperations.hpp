#ifndef FILEOPERATIONS_HPP
#define FILEOPERATIONS_HPP

#include <filesystem>
#include <optional>
#include <string>

/**
 * @brief Copies a file from source to destination.
 *
 * @param source The source file path.
 * @param destination The destination file path.
 * @return std::optional<std::string> An optional string containing an error message if any, or an empty optional if successful.
 */
std::optional<std::string> custom_copy_file(const std::filesystem::path& source, const std::filesystem::path& destination);

#endif // FILEOPERATIONS_HPP
