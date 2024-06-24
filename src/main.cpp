#include "lua_bindings/chdir.hpp"
#include "lua_bindings/currentDir.hpp"
#include "lua_bindings/deepCopyTable.hpp"
#include "lua_bindings/fileExists.hpp"
#include "lua_bindings/fileSize.hpp"
#include "lua_bindings/fileUtils.hpp"
#include "lua_bindings/helloThere.hpp"
#include "lua_bindings/listFiles.hpp"
#include "lua_bindings/memoryUtils.hpp"
#include "lua_bindings/mergeTables.hpp"
#include "lua_bindings/mkdir.hpp"
#include "lua_bindings/rmdir.hpp"
#include "lua_bindings/split.hpp"
#include "project_core/loadLuaFile.hpp"
#include "project_core/zip_utils.hpp"
#include "project_core/create_executable.hpp"
#include "project_core/help.hpp"
#include <iostream>
#include <lua.hpp>
#include <filesystem>
#include <physfs.h>

namespace fs = std::filesystem;

void register_luapilot(lua_State *L) {
    // Crée une nouvelle table Lua
    lua_newtable(L);

    // Lie la fonction C++ lua_chdir à la table sous le nom "chdir"
    lua_pushcfunction(L, lua_chdir);
    lua_setfield(L, -2, "chdir");

    // Lie la fonction C++ lua_chdir à la table sous le nom "chdir"
    lua_pushcfunction(L, lua_currentDir);
    lua_setfield(L, -2, "currentDir");

    // Lie la fonction C++ lua_deepCopyTable à la table sous le nom "deepCopy"
    lua_pushcfunction(L, lua_deepCopyTable);
    lua_setfield(L, -2, "deepCopyTable");

    // Lie la fonction C++ lua_fileExists à la table sous le nom "fileExists"
    lua_pushcfunction(L, lua_fileExists);
    lua_setfield(L, -2, "fileExists");

    // Lie la fonction C++ lua_fileSize à la table sous le nom "fileSize"
    lua_pushcfunction(L, lua_fileSize);
    lua_setfield(L, -2, "fileSize");

    // Lie la fonction C++ lua_getBasename à la table sous le nom "getBasename"
    lua_pushcfunction(L, lua_getBasename);
    lua_setfield(L, -2, "getBasename");

    // Lie la fonction C++ lua_getExtension à la table sous le nom "getExtension"
    lua_pushcfunction(L, lua_getExtension);
    lua_setfield(L, -2, "getExtension");

    // Lie la fonction C++ lua_getFilename à la table sous le nom "getFilename"
    lua_pushcfunction(L, lua_getFilename);
    lua_setfield(L, -2, "getFilename");

    // Lie la fonction C++ lua_getPath à la table sous le nom "getPath"
    lua_pushcfunction(L, lua_getPath);
    lua_setfield(L, -2, "getPath");

    // Lie la fonction C++ lua_helloThere à la table sous le nom "helloThere"
    lua_pushcfunction(L, lua_helloThere);
    lua_setfield(L, -2, "helloThere");

    // Lie la fonction C++ lua_listFiles à la table sous le nom "listFiles"
    lua_pushcfunction(L, lua_listFiles);
    lua_setfield(L, -2, "listFiles");

    // Lie la fonction C++ lua_getMemoryUsage à la table sous le nom "getMemoryUsage"
    lua_pushcfunction(L, lua_getMemoryUsage);
    lua_setfield(L, -2, "getMemoryUsage");

    lua_pushcfunction(L, lua_mergeTables);
    lua_setfield(L, -2, "mergeTables");

    // Lie la fonction C++ lua_mkdir à la table sous le nom "mkdir"
    lua_pushcfunction(L, lua_mkdir);
    lua_setfield(L, -2, "mkdir");

    // Lie la fonction C++ lua_rmdir à la table sous le nom "rmdir"
    lua_pushcfunction(L, lua_rmdir);
    lua_setfield(L, -2, "rmdir");

    // Lie la fonction C++ lua_rmdir à la table sous le nom "rmdir"
    lua_pushcfunction(L, lua_rmdir_all);
    lua_setfield(L, -2, "rmdir_all");

    // Lie la fonction C++ split à la table sous le nom "split"
    lua_pushcfunction(L, lua_split);
    lua_setfield(L, -2, "split");

    // Enregistre la table globale "luapilot" dans l'état Lua
    lua_setglobal(L, "luapilot");
}

int main(int argc, char *argv[]) {
    // Vérifie si une option d'aide est présente parmi les arguments
    for (int i = 1; i < argc; ++i) {
        std::string option = argv[i];
        if (option == "--help" || option == "-h") {
            printHelp(); // Affiche l'aide
            return 0;
        }
    }

    // Si aucun argument n'est fourni, initialise PhysFS et exécute le script Lua
    if (argc < 2) {
        // Initialiser PhysFS avec le chemin de l'exécutable
        if (!initPhysFS(argv[0])) {
            return 1;
        }

        // Nom de l'archive ZIP à monter
        const char *archive = "embedded.zip";

        // Chemin du fichier main.lua dans l'archive montée
        std::string mainLuaPath = "main.lua";

        // Monter l'archive ZIP intégrée
        if (!mountZipFile(argv[0])) {
            return 1;
        }

        // Vérifie si le fichier main.lua existe dans l'archive ZIP montée
        if (!PHYSFS_exists(mainLuaPath.c_str())) {
            std::cerr << "Erreur : main.lua introuvable dans l'archive ZIP" << std::endl;
            PHYSFS_deinit(); // Désinitialise PhysFS
            return 1;
        }

        // Crée un nouvel état Lua et ouvre les bibliothèques Lua standard
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);

        // Enregistre les fonctions Lua personnalisées
        register_luapilot(L);

        // Lecture du fichier main.lua depuis l'archive ZIP
        char *fileData;
        PHYSFS_File *file = PHYSFS_openRead(mainLuaPath.c_str());
        PHYSFS_sint64 fileSize = PHYSFS_fileLength(file);
        fileData = new char[fileSize];
        PHYSFS_readBytes(file, fileData, fileSize);
        PHYSFS_close(file);

        // Charge et exécute le script Lua
        if (luaL_loadbuffer(L, fileData, fileSize, mainLuaPath.c_str()) || lua_pcall(L, 0, LUA_MULTRET, 0)) {
            std::cerr << "Erreur : " << lua_tostring(L, -1) << std::endl;
            delete[] fileData;
            lua_close(L);
            PHYSFS_deinit();
            return 1;
        }

        // Libère les ressources
        delete[] fileData;
        lua_close(L);
        PHYSFS_deinit();
        return 0;
    }

    // Gestion des options de ligne de commande
    std::string option = argv[1];

    // Option pour créer un exécutable
    if (option == "--create-exe" || option == "-c") {
        if (argc != 4) {
            std::cerr << "Usage : " << argv[0] << " --create-exe <dir> <output>" << std::endl;
            return 1;
        }
        std::string dir = argv[2];
        std::string output = argv[3];
        createExecutableWithDir(dir, output); // Crée l'exécutable avec le répertoire spécifié
        return 0;
    }

    // Option pour afficher l'aide
    if (option == "--help" || option == "-h") {
        printHelp();
        return 0;
    }

    // Chemin vers le script Lua à exécuter
    std::string path = argv[1];
    std::string mainLuaPath = fs::path(path) / "main.lua";

    // Vérifie si le fichier main.lua existe dans le répertoire spécifié
    if (!fs::exists(mainLuaPath)) {
        std::cerr << "Erreur : main.lua introuvable dans le répertoire " << path << std::endl;
        return 1;
    }

    // Crée un nouvel état Lua et ouvre les bibliothèques Lua standard
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    // Enregistre les fonctions Lua personnalisées
    register_luapilot(L);

    // Charge et exécute le script Lua
    loadLuaFile(L, mainLuaPath);

    // Ferme l'état Lua
    lua_close(L);
    return 0;
}
