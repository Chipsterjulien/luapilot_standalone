#include "zip_utils.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <array>
#include <algorithm>
#include <physfs.h>
#include <zip.h>
#include <system_error>

namespace fs = std::filesystem;

/**
 * @brief Creates a ZIP file from the specified directory.
 *
 * This function traverses the given directory and adds all regular files to a ZIP archive.
 * It ensures that the executable itself is not included in the ZIP file.
 *
 * @param dir The directory to be zipped.
 * @param zipFileName The name of the resulting ZIP file.
 * @return true if the ZIP file was created successfully, false otherwise.
 */
bool createZipFromDirectory(const std::string &dir, const std::string &zipFileName) {
    int error;
    zip_t *zip = zip_open(zipFileName.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!zip) {
        std::cerr << "Error creating ZIP file: " << zipFileName << " (" << std::system_error(error, std::generic_category()).what() << ")" << std::endl;
        return false;
    }

    // Get the path of the current executable
    std::string exePath = fs::read_symlink("/proc/self/exe").string();

    for (const auto &entry : fs::recursive_directory_iterator(dir)) {
        if (fs::is_regular_file(entry)) {
            std::string filePath = fs::canonical(entry.path()).string();
            // Ensure the executable itself is not added to the ZIP
            if (filePath == exePath) {
                continue;
            }

            std::string relativePath = fs::relative(entry.path(), dir).string();

            zip_source_t *source = zip_source_file(zip, filePath.c_str(), 0, 0);
            if (source == nullptr) {
                std::cerr << "Error adding file to ZIP: " << filePath << std::endl;
                zip_close(zip);
                return false;
            }

            zip_int64_t fileIndex = zip_file_add(zip, relativePath.c_str(), source, ZIP_FL_OVERWRITE);
            if (fileIndex < 0) {
                std::cerr << "Error adding file to ZIP: " << filePath << std::endl;
                zip_source_free(source);
                zip_close(zip);
                return false;
            }

            // Set the compression level to maximum (9)
            if (zip_set_file_compression(zip, fileIndex, ZIP_CM_DEFLATE, 9) < 0) {
                std::cerr << "Error setting compression level for file: " << filePath << std::endl;
                zip_close(zip);
                return false;
            }
        }
    }

    zip_close(zip);

//    // Save the ZIP file for debugging purposes
//    if (fs::exists(zipFileName)) {
//        std::string debugZipFileName = fs::path(zipFileName).parent_path() / ("debug_" + fs::path(zipFileName).filename().string());
//        fs::copy(zipFileName, debugZipFileName, fs::copy_options::overwrite_existing);
//    } else {
//        std::cerr << "Error: ZIP file does not exist for debugging: " << zipFileName << std::endl;
//    }

    return true;
}

/**
 * @brief Merges an executable file and a ZIP file into a single output file.
 *
 * This function concatenates the content of the executable file and the ZIP file into a single output file.
 *
 * @param exe The path to the executable file.
 * @param zip The path to the ZIP file.
 * @param output The path to the output file.
 */
void mergeFiles(const std::string &exe, const std::string &zip, const std::string &output) {
    std::ifstream exeFile(exe, std::ios::binary);
    std::ifstream zipFile(zip, std::ios::binary);
    std::ofstream outFile(output, std::ios::binary);

    outFile << exeFile.rdbuf() << zipFile.rdbuf();

    std::cout << "Successfully created executable: " << output << std::endl;
}

/**
 * @brief Extracts an embedded ZIP archive from an executable file.
 *
 * This function searches for an embedded ZIP archive within the specified executable file and extracts it to the given output directory.
 *
 * @param exePath The path to the executable file containing the embedded ZIP archive.
 * @param outputDir The directory to extract the ZIP archive to.
 * @return true if the ZIP archive was extracted successfully, false otherwise.
 */
bool extractEmbeddedZip(const std::string &exePath, const std::string &outputDir) {
    std::ifstream exeFile(exePath, std::ios::binary);
    if (!exeFile) {
        std::cerr << "Error: unable to open executable file " << exePath << std::endl;
        return false;
    }

    exeFile.seekg(0, std::ios::end);
    std::streamoff exeSize = exeFile.tellg();
    exeFile.seekg(0, std::ios::beg);

    std::vector<char> buffer(exeSize);
    exeFile.read(buffer.data(), exeSize);

    const std::array<char, 4> endOfCentralDirSignature = {0x50, 0x4b, 0x05, 0x06};
    auto it = std::search(buffer.rbegin(), buffer.rend(), endOfCentralDirSignature.rbegin(), endOfCentralDirSignature.rend());

    if (it == buffer.rend()) {
        std::cerr << "Error: no embedded ZIP file found" << std::endl;
        return false;
    }

    std::streamoff zipStart = exeSize - std::distance(buffer.rbegin(), it) - endOfCentralDirSignature.size();
    exeFile.seekg(zipStart);

    std::string zipFileName = outputDir + "/embedded.zip";
    std::ofstream zipFile(zipFileName, std::ios::binary);
    zipFile.write(buffer.data() + zipStart, exeSize - zipStart);
    zipFile.close();

    int error;
    zip_t *zip = zip_open(zipFileName.c_str(), ZIP_RDONLY, &error);
    if (!zip) {
        std::cerr << "Error: unable to open embedded ZIP file: " << zipFileName << " (error code: " << error << ")" << std::endl;
        return false;
    }

    for (int i = 0; i < zip_get_num_entries(zip, 0); ++i) {
        struct zip_stat st;
        zip_stat_index(zip, i, 0, &st);

        zip_file *file = zip_fopen_index(zip, i, 0);
        if (!file) {
            continue;
        }

        std::string filePath = outputDir + "/" + st.name;
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error: unable to create file " << filePath << std::endl;
            zip_fclose(file);
            zip_close(zip);
            return false;
        }

        char buffer[8192];
        zip_int64_t bytesRead;
        while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0) {
            outFile.write(buffer, bytesRead);
        }

        zip_fclose(file);
        outFile.close();
    }

    zip_close(zip);
    fs::remove(zipFileName);
    return true;
}

/**
 * @brief Initializes the PhysicsFS library.
 *
 * This function initializes the PhysicsFS library using the given argument.
 *
 * @param argv0 The argument to pass to PHYSFS_init.
 * @return true if the initialization was successful, false otherwise.
 */
bool initPhysFS(const char *argv0) {
    if (!PHYSFS_init(argv0)) {
        PHYSFS_ErrorCode errorCode = PHYSFS_getLastErrorCode();
        std::cerr << "Error initializing PhysFS: " << PHYSFS_getErrorByCode(errorCode) << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Mounts a ZIP file using the PhysicsFS library.
 *
 * This function mounts the specified ZIP file using the PhysicsFS library.
 *
 * @param archive The path to the ZIP file to mount.
 * @return true if the ZIP file was mounted successfully, false otherwise.
 */
bool mountZipFile(const char *archive) {
    if (!PHYSFS_mount(archive, NULL, 1)) {
        PHYSFS_ErrorCode errorCode = PHYSFS_getLastErrorCode();
        std::cerr << "Error mounting archive " << archive << ": " << PHYSFS_getErrorByCode(errorCode) << std::endl;
        return false;
    }
    return true;
}
