#include "fileOperations.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <system_error>

/**
 * @brief Copies a file from source to destination.
 *
 * @param source The source file path.
 * @param destination The destination file path.
 * @return std::optional<std::string> An optional string containing an error message if any, or an empty optional if successful.
 */
std::optional<std::string> custom_copy_file(const std::filesystem::path& source, const std::filesystem::path& destination) {
    std::error_code ec;

    // Check if source file exists
    if (!std::filesystem::exists(source, ec)) {
        return "Source file does not exist: " + source.string();
    }

    // Check if destination path is a directory
    if (std::filesystem::is_directory(destination, ec)) {
        return "Destination path is a directory: " + destination.string();
    }

    // Copy the file
    std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
        return "Error copying file: " + ec.message();
    }

    return std::nullopt;
}
