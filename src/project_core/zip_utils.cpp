#include "zip_utils.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <array>
#include <algorithm>
#include <physfs.h>
#include <zip.h>

namespace fs = std::filesystem;

bool createZipFromDirectory(const std::string &dir, const std::string &zipFileName)
{
    int error;
    zip_t *zip = zip_open(zipFileName.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!zip)
    {
        std::cerr << "Erreur lors de la création du fichier ZIP : " << zipFileName << std::endl;
        return false;
    }

    for (const auto &entry : fs::recursive_directory_iterator(dir))
    {
        if (fs::is_regular_file(entry))
        {
            std::string filePath = entry.path().string();
            std::string relativePath = fs::relative(entry.path(), dir).string();

            zip_source_t *source = zip_source_file(zip, filePath.c_str(), 0, 0);
            if (source == nullptr)
            {
                std::cerr << "Erreur lors de l'ajout du fichier au ZIP : " << filePath << std::endl;
                zip_close(zip);
                return false;
            }

            zip_int64_t fileIndex = zip_file_add(zip, relativePath.c_str(), source, ZIP_FL_OVERWRITE);
            if (fileIndex < 0)
            {
                std::cerr << "Erreur lors de l'ajout du fichier au ZIP : " << filePath << std::endl;
                zip_source_free(source);
                zip_close(zip);
                return false;
            }

            // Set the compression level to maximum (9)
            if (zip_set_file_compression(zip, fileIndex, ZIP_CM_DEFLATE, 9) < 0)
            {
                std::cerr << "Erreur lors de la définition du niveau de compression pour le fichier : " << filePath << std::endl;
                zip_close(zip);
                return false;
            }
        }
    }

    zip_close(zip);
    return true;
}

void mergeFiles(const std::string &exe, const std::string &zip, const std::string &output)
{
    std::ifstream exeFile(exe, std::ios::binary);
    std::ifstream zipFile(zip, std::ios::binary);
    std::ofstream outFile(output, std::ios::binary);

    outFile << exeFile.rdbuf() << zipFile.rdbuf();

    std::cout << "Exécutable créé avec succès : " << output << std::endl;
}

bool extractEmbeddedZip(const std::string &exePath, const std::string &outputDir)
{
    std::ifstream exeFile(exePath, std::ios::binary);
    if (!exeFile)
    {
        std::cerr << "Erreur : impossible d'ouvrir le fichier exécutable " << exePath << std::endl;
        return false;
    }

    exeFile.seekg(0, std::ios::end);
    std::streamoff exeSize = exeFile.tellg();
    exeFile.seekg(0, std::ios::beg);

    std::vector<char> buffer(exeSize);
    exeFile.read(buffer.data(), exeSize);

    const std::array<char, 4> endOfCentralDirSignature = {0x50, 0x4b, 0x05, 0x06};
    auto it = std::search(buffer.rbegin(), buffer.rend(), endOfCentralDirSignature.rbegin(), endOfCentralDirSignature.rend());

    if (it == buffer.rend())
    {
        std::cerr << "Erreur : aucun fichier ZIP intégré trouvé" << std::endl;
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
    if (!zip)
    {
        std::cerr << "Erreur : impossible d'ouvrir le fichier ZIP intégré : " << zipFileName << " (code d'erreur : " << error << ")" << std::endl;
        return false;
    }

    for (int i = 0; i < zip_get_num_entries(zip, 0); ++i)
    {
        struct zip_stat st;
        zip_stat_index(zip, i, 0, &st);

        zip_file *file = zip_fopen_index(zip, i, 0);
        if (!file)
            continue;

        std::string filePath = outputDir + "/" + st.name;
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile)
        {
            std::cerr << "Erreur : impossible de créer le fichier " << filePath << std::endl;
            zip_fclose(file);
            zip_close(zip);
            return false;
        }

        char buffer[8192];
        zip_int64_t bytesRead;
        while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0)
        {
            outFile.write(buffer, bytesRead);
        }

        zip_fclose(file);
        outFile.close();
    }

    zip_close(zip);
    fs::remove(zipFileName);
    return true;
}

bool initPhysFS(const char *argv0)
{
    if (!PHYSFS_init(argv0))
    {
        PHYSFS_ErrorCode errorCode = PHYSFS_getLastErrorCode();
        std::cerr << "Erreur lors de l'initialisation de PhysFS: " << PHYSFS_getErrorByCode(errorCode) << std::endl;
        return false;
    }
    return true;
}

bool mountZipFile(const char *archive)
{
    if (!PHYSFS_mount(archive, NULL, 1))
    {
        PHYSFS_ErrorCode errorCode = PHYSFS_getLastErrorCode();
        std::cerr << "Erreur lors du montage de l'archive " << archive << ": " << PHYSFS_getErrorByCode(errorCode) << std::endl;
        return false;
    }
    return true;
}
