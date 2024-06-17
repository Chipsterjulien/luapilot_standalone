#include "create_executable.hpp"
#include "zip_utils.hpp"
#include "executable_path.hpp"
#include <iostream>
#include <filesystem>
#include <sys/stat.h>  // Pour chmod

namespace fs = std::filesystem;

void createExecutableWithDir(const std::string &dir, const std::string &output) {
    std::string mainLuaPath = fs::path(dir) / "main.lua";
    if (!fs::exists(mainLuaPath)) {
        std::cerr << "Erreur : main.lua introuvable dans le répertoire " << dir << std::endl;
        return;
    }

    std::string zipFileName = fs::path(dir) / "temp.zip";
    if (!createZipFromDirectory(dir, zipFileName)) {
        std::cerr << "Erreur : échec de la création du fichier ZIP." << std::endl;
        return;
    }

    std::string exe = getExecutablePath(); // Utilisation de getExecutablePath pour obtenir le chemin du binaire

    mergeFiles(exe, zipFileName, output);

    fs::remove(zipFileName);

    // Rendre le fichier de sortie exécutable
    if (chmod(output.c_str(), 0755) != 0) {
        std::cerr << "Erreur : impossible de rendre le fichier exécutable " << output << std::endl;
    }
}
