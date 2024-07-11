#ifndef FILE_OPERATIONS_HPP
#define FILE_OPERATIONS_HPP

#include <string>
#include <utility> // for std::pair

/**
 * @brief Function to copy a single file
 *
 * This function takes two strings representing the source path and the destination path,
 * and copies the file, including its metadata.
 *
 * @param src_path The path of the source file
 * @param dest_path The path of the destination file
 * @return A pair containing a boolean (true if the copy operation was successful, false otherwise)
 *         and a string with the error message if any.
 */
std::pair<bool, std::string> copy_file(const std::string& src_path, const std::string& dest_path);

/**
 * @brief Function to create a symbolic link
 *
 * This function takes two strings representing the target path and the link path,
 * and creates the symbolic link.
 *
 * @param target_path The target of the symbolic link
 * @param link_path The path of the symbolic link
 * @return A pair containing a boolean (true if the link creation was successful, false otherwise)
 *         and a string with the error message if any.
 */
std::pair<bool, std::string> create_symlink(const std::string& target_path, const std::string& link_path);

#endif // FILE_OPERATIONS_HPP
