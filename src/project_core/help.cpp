#include "help.hpp"
#include <iostream>

void printHelp() {
    std::cout << "Usage: luapilot [options] <arguments>\n"
              << "Options:\n"
              << "  -h, --help           Affiche ce message d'aide.\n"
              << "  -c, --create-exe     Crée un exécutable en incluant un répertoire.\n"
              << "\n  Utilisation : -c <directory> <output>\n"
              << "  <directory>          Répertoire contenant main.lua à exécuter.\n"
              << "  <output>             Nom du futur exécutable.\n"
              << "\n\nSi aucun argument n'est fourni, le programme\n"
              << "                       recherche et exécute main.lua intégré dans l'exécutable.\n"
              << "\nExemples:\n"
              << "  luapilot -c my_scripts luapilot_with_scripts\n"
              << "      Crée un exécutable nommé 'luapilot_with_scripts' en incluant\n"
              << "      tous les fichiers du répertoire 'my_scripts'.\n"
              << "\n"
              << "  luapilot my_directory\n"
              << "      Exécute le fichier 'main.lua' situé dans 'my_directory'.\n"
              << "\n"
              << "  luapilot\n"
              << "      Si aucun argument n'est fourni, le programme recherche et exécute\n"
              << "      un fichier 'main.lua' intégré dans l'exécutable lui-même.\n"
              << std::endl;
}
