#include "zip_utils.hpp"
#include "miniz.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <cerrno>

namespace fs = std::filesystem;

bool createZipFromDirectory(const std::string &dir, const std::string &zipFileName)
{
    mz_zip_archive zip = {};

    if (!mz_zip_writer_init_file(&zip, zipFileName.c_str(), 0))
    {
        std::cerr << "Error creating ZIP file: " << zipFileName << std::endl;
        return false;
    }

    for (const auto &entry : fs::recursive_directory_iterator(dir))
    {
        if (!fs::is_regular_file(entry))
            continue;

        std::string relativePath = fs::relative(entry.path(), dir).string();

        if (!mz_zip_writer_add_file(&zip, relativePath.c_str(),
                                    entry.path().string().c_str(),
                                    nullptr, 0, MZ_BEST_COMPRESSION))
        {
            std::cerr << "Error adding to ZIP: " << entry.path() << std::endl;
            mz_zip_writer_end(&zip);
            return false;
        }
    }

    if (!mz_zip_writer_finalize_archive(&zip))
    {
        std::cerr << "Error finalizing ZIP" << std::endl;
        mz_zip_writer_end(&zip);
        return false;
    }
    mz_zip_writer_end(&zip);
    return true;
}

void mergeFiles(const std::string &exe, const std::string &zip, const std::string &output)
{
    std::ifstream exeFile(exe, std::ios::binary);
    if (!exeFile)
    {
        throw std::system_error(errno, std::generic_category(), "open " + exe);
    }
    std::ifstream zipFile(zip, std::ios::binary);
    if (!zipFile)
    {
        throw std::system_error(errno, std::generic_category(), "open " + zip);
    }
    std::ofstream outFile(output, std::ios::binary);
    if (!outFile)
    {
        throw std::system_error(errno, std::generic_category(), "create " + output);
    }

    outFile << exeFile.rdbuf() << zipFile.rdbuf();
    if (!outFile)
    {
        throw std::system_error(errno, std::generic_category(), "write " + output);
    }
    std::cout << "Successfully created executable: " << output << std::endl;
}

std::optional<std::vector<char>> readEmbeddedFile(const std::string &exePath,
                                                  const std::string &archivePath)
{
    mz_zip_archive zip = {};

    // miniz scanne depuis la fin du fichier pour trouver l'EOCD, donc un zip
    // appendé à un exécutable est trouvé automatiquement.
    // Cette fonction est silencieuse : c'est aux callers de logger leurs erreurs
    // (le searcher Lua n'a pas besoin de spammer stderr à chaque require).
    if (!mz_zip_reader_init_file(&zip, exePath.c_str(), 0))
    {
        return std::nullopt;
    }

    int idx = mz_zip_reader_locate_file(&zip, archivePath.c_str(), nullptr, 0);
    if (idx < 0)
    {
        mz_zip_reader_end(&zip);
        return std::nullopt;
    }

    mz_zip_archive_file_stat stat;
    if (!mz_zip_reader_file_stat(&zip, idx, &stat))
    {
        mz_zip_reader_end(&zip);
        return std::nullopt;
    }

    std::vector<char> data(static_cast<size_t>(stat.m_uncomp_size));
    if (!mz_zip_reader_extract_to_mem(&zip, idx, data.data(), data.size(), 0))
    {
        mz_zip_reader_end(&zip);
        return std::nullopt;
    }

    mz_zip_reader_end(&zip);
    return data;
}
