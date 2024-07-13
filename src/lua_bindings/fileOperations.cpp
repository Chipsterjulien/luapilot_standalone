#include "fileOperations.hpp"
#include <system_error>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <sys/stat.h> // for struct stat, stat, lstat, chmod
#include <unistd.h> // for readlink, symlink
#include <limits.h> // for PATH_MAX

/**
 * @brief Copies a file from the source path to the destination path.
 *
 * This function reads the contents of the source file and writes them to the destination file.
 * It also copies the file permissions from the source file to the destination file.
 *
 * @param src_path The path of the source file.
 * @param dest_path The path of the destination file.
 * @return A string containing an error message if any, or an empty string if successful.
 */
std::string copy_file(const std::string& src_path, const std::string& dest_path) {
    struct stat st;
    if (stat(src_path.c_str(), &st) != 0) {
        std::error_code ec(errno, std::generic_category());
        return ec.message();
    }

    FILE* src = std::fopen(src_path.c_str(), "rb");
    if (!src) {
        std::error_code ec(errno, std::generic_category());
        return ec.message();
    }

    FILE* dest = std::fopen(dest_path.c_str(), "wb");
    if (!dest) {
        std::fclose(src);
        std::error_code ec(errno, std::generic_category());
        return ec.message();
    }

    char buffer[BUFSIZ];
    size_t size;
    while ((size = std::fread(buffer, 1, BUFSIZ, src))) {
        if (std::fwrite(buffer, 1, size, dest) != size) {
            std::fclose(src);
            std::fclose(dest);
            std::error_code ec(errno, std::generic_category());
            return ec.message();
        }
    }

    std::fclose(src);
    std::fclose(dest);

    // Copy the file permissions
    if (chmod(dest_path.c_str(), st.st_mode) != 0) {
        std::error_code ec(errno, std::generic_category());
        return ec.message();
    }

    return "";
}

/**
 * @brief Creates a symbolic link at the specified path pointing to the target path.
 *
 * This function creates a symbolic link from the link_path to the target_path.
 *
 * @param target_path The path that the symbolic link will point to.
 * @param link_path The path where the symbolic link will be created.
 * @return A string containing an error message if any, or an empty string if successful.
 */
std::string create_symlink(const std::string& target_path, const std::string& link_path) {
    if (symlink(target_path.c_str(), link_path.c_str()) != 0) {
        std::error_code ec(errno, std::generic_category());
        return ec.message();
    }
    return "";
}
