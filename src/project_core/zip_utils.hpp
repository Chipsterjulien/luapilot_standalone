#ifndef ZIP_UTILS_HPP
#define ZIP_UTILS_HPP

#include <optional>
#include <string>
#include <vector>

/**
 * @brief Creates a ZIP archive from all regular files under `dir`.
 * @return true on success.
 */
bool createZipFromDirectory(const std::string &dir, const std::string &zipFileName);

/**
 * @brief Concatenates exe + zip into output.
 * @throws std::system_error on I/O failure.
 */
void mergeFiles(const std::string &exe, const std::string &zip, const std::string &output);

/**
 * @brief Opens the zip appended to `exePath`, extracts the named entry into memory.
 *        This function is silent: callers are responsible for reporting failures.
 * @return The file contents, or std::nullopt on failure.
 */
std::optional<std::vector<char>> readEmbeddedFile(const std::string &exePath,
                                                  const std::string &archivePath);

#endif // ZIP_UTILS_HPP