#include "create_executable.hpp"
#include "zip_utils.hpp"
#include "executable_path.hpp"
#include <iostream>
#include <filesystem>
#include <system_error>
#include <sys/stat.h>

namespace fs = std::filesystem;

/**
 * @brief Creates an executable file that includes a directory with a main.lua script.
 *
 * This function creates a ZIP file from the specified directory, merges it with the current executable,
 * and makes the resulting file executable.
 *
 * @param dir The directory containing the main.lua script and other files to include.
 * @param output The path to the output executable file.
 */
void createExecutableWithDir(const std::string &dir, const std::string &output) {
    fs::path mainLuaPath = fs::path(dir) / "main.lua";

    // Check if main.lua exists in the specified directory
    if (!fs::exists(mainLuaPath)) {
        std::cerr << "Error: main.lua not found in the directory " << dir << std::endl;
        return;
    }

    fs::path zipFileName = fs::path(dir) / "temp.zip";

    // Create a ZIP file from the directory
    if (!createZipFromDirectory(dir, zipFileName.string())) {
        std::cerr << "Error: failed to create ZIP file." << std::endl;
        return;
    }

    // Get the path of the current executable
    std::string exe = getExecutablePath();

    // Merge the executable with the ZIP file
    try {
        mergeFiles(exe, zipFileName.string(), output);
    } catch (const std::system_error& e) {
        std::cerr << "Error: failed to merge files - " << e.what() << std::endl;
        return;
    }

    // Remove the temporary ZIP file
    std::error_code ec;
    fs::remove(zipFileName, ec);
    if (ec) {
        std::cerr << "Error: failed to remove temporary ZIP file - " << ec.message() << std::endl;
    }

    // Make the output file executable
    if (chmod(output.c_str(), 0755) != 0) {
        std::cerr << "Error: failed to make the file executable " << output << std::endl;
    }
}
