#include "lua_bindings/fileOperations.hpp"
#include <cstdio> // for std::fopen, std::fread, std::fwrite, std::fclose
#include <cerrno> // for errno
#include <cstring> // for strerror
#include <sys/stat.h> // for struct stat, stat, lstat, chmod
#include <unistd.h> // for readlink, symlink
#include <limits.h> // for PATH_MAX

std::pair<bool, std::string> copy_file(const std::string& src_path, const std::string& dest_path) {
    struct stat st;
    if (stat(src_path.c_str(), &st) != 0) {
        return {false, strerror(errno)};
    }

    FILE* src = std::fopen(src_path.c_str(), "rb");
    if (!src) {
        return {false, strerror(errno)};
    }

    FILE* dest = std::fopen(dest_path.c_str(), "wb");
    if (!dest) {
        std::fclose(src);
        return {false, strerror(errno)};
    }

    char buffer[BUFSIZ];
    size_t size;
    while ((size = std::fread(buffer, 1, BUFSIZ, src))) {
        if (std::fwrite(buffer, 1, size, dest) != size) {
            std::fclose(src);
            std::fclose(dest);
            return {false, strerror(errno)};
        }
    }

    std::fclose(src);
    std::fclose(dest);

    // Copy the file permissions
    if (chmod(dest_path.c_str(), st.st_mode) != 0) {
        return {false, strerror(errno)};
    }

    return {true, ""};
}

std::pair<bool, std::string> create_symlink(const std::string& target_path, const std::string& link_path) {
    if (symlink(target_path.c_str(), link_path.c_str()) != 0) {
        return {false, strerror(errno)};
    }
    return {true, ""};
}
