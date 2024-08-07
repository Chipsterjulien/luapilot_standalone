#include "lua_bindings/attributes.hpp"
#include "lua_bindings/chdir.hpp"
#include "lua_bindings/copy.hpp"
#include "lua_bindings/copyTree.hpp"
#include "lua_bindings/currentDir.hpp"
#include "lua_bindings/deepCopyTable.hpp"
#include "lua_bindings/fileExists.hpp"
#include "lua_bindings/fileSize.hpp"
#include "lua_bindings/fileUtils.hpp"
#include "lua_bindings/find.hpp"
#include "lua_bindings/helloThere.hpp"
#include "lua_bindings/isdir.hpp"
#include "lua_bindings/isfile.hpp"
#include "lua_bindings/link.hpp"
#include "lua_bindings/listFiles.hpp"
#include "lua_bindings/md5.hpp"
#include "lua_bindings/memoryUtils.hpp"
#include "lua_bindings/mergeTables.hpp"
#include "lua_bindings/mkdir.hpp"
#include "lua_bindings/mode.hpp"
#include "lua_bindings/moveTree.hpp"
#include "lua_bindings/joinPath.hpp"
#include "lua_bindings/rename.hpp"
#include "lua_bindings/remove.hpp"
#include "lua_bindings/rmdir.hpp"
#include "lua_bindings/sha1.hpp"
#include "lua_bindings/sha3_256.hpp"
#include "lua_bindings/sha3_512.hpp"
#include "lua_bindings/sha256.hpp"
#include "lua_bindings/sha512.hpp"
#include "lua_bindings/sleep.hpp"
#include "lua_bindings/split.hpp"
#include "lua_bindings/symlinkattr.hpp"
#include "lua_bindings/touch.hpp"
#include "lua_bindings/fileIterator.hpp"

#include "project_core/loadLuaFile.hpp"
#include "project_core/zip_utils.hpp"
#include "project_core/create_executable.hpp"
#include "project_core/help.hpp"

#include <iostream>
#include <lua.hpp>
#include <filesystem>
#include <physfs.h>

namespace fs = std::filesystem;

/**
 * @brief Register LuaPilot functions to Lua state.
 * @param L Lua state.
 */
void register_luapilot(lua_State *L) {
    // Crée une nouvelle table Lua
    lua_newtable(L);

    lua_pushcfunction(L, lua_setattr);
    lua_setfield(L, -2, "setAttributes");

    lua_pushcfunction(L, lua_getattr);
    lua_setfield(L, -2, "getAttributes");

    lua_pushcfunction(L, lua_chdir);
    lua_setfield(L, -2, "chdir");

    lua_pushcfunction(L, lua_copy_file);
    lua_setfield(L, -2, "copy");

    lua_pushcfunction(L, lua_copyTree);
    lua_setfield(L, -2, "copyTree");

    lua_pushcfunction(L, lua_currentDir);
    lua_setfield(L, -2, "currentDir");

    lua_pushcfunction(L, lua_deepCopyTable);
    lua_setfield(L, -2, "deepCopyTable");

    lua_pushcfunction(L, lua_fileExists);
    lua_setfield(L, -2, "fileExists");

    lua_pushcfunction(L, lua_fileSize);
    lua_setfield(L, -2, "fileSize");

    lua_pushcfunction(L, lua_find);
    lua_setfield(L, -2, "find");

    lua_pushcfunction(L, lua_getBasename);
    lua_setfield(L, -2, "getBasename");

    lua_pushcfunction(L, lua_getExtension);
    lua_setfield(L, -2, "getExtension");

    lua_pushcfunction(L, lua_getFilename);
    lua_setfield(L, -2, "getFilename");

    lua_pushcfunction(L, lua_getMemoryUsage);
    lua_setfield(L, -2, "getMemoryUsage");

    lua_pushcfunction(L, lua_getDetailedMemoryUsage);
    lua_setfield(L, -2, "getDetailedMemoryUsage");

    lua_pushcfunction(L, lua_getPath);
    lua_setfield(L, -2, "getPath");

    lua_pushcfunction(L, lua_helloThere);
    lua_setfield(L, -2, "helloThere");

    lua_pushcfunction(L, lua_isDir);
    lua_setfield(L, -2, "isdir");

    lua_pushcfunction(L, lua_isFile);
    lua_setfield(L, -2, "isfile");

    lua_pushcfunction(L, lua_link);
    lua_setfield(L, -2, "link");

    lua_pushcfunction(L, lua_listFiles);
    lua_setfield(L, -2, "listFiles");

    lua_pushcfunction(L, lua_md5sum);
    lua_setfield(L, -2, "md5sum");

    lua_pushcfunction(L, lua_mergeTables);
    lua_setfield(L, -2, "mergeTables");

    lua_pushcfunction(L, lua_mkdir);
    lua_setfield(L, -2, "mkdir");

    lua_pushcfunction(L, lua_moveTree);
    lua_setfield(L, -2, "moveTree");

    lua_pushcfunction(L, lua_joinPath);
    lua_setfield(L, -2, "joinPath");

    lua_pushcfunction(L, lua_remove_file);
    lua_setfield(L, -2, "remove");

    lua_pushcfunction(L, lua_rename);
    lua_setfield(L, -2, "rename");

    lua_pushcfunction(L, lua_rmdir);
    lua_setfield(L, -2, "rmdir");

    lua_pushcfunction(L, lua_rmdir_all);
    lua_setfield(L, -2, "rmdirAll");

    lua_pushcfunction(L, lua_setmode);
    lua_setfield(L, -2, "setMode");

    lua_pushcfunction(L, lua_getmode);
    lua_setfield(L, -2, "getMode");

    lua_pushcfunction(L, lua_sha1sum);
    lua_setfield(L, -2, "sha1sum");

    lua_pushcfunction(L, lua_sha3_256sum);
    lua_setfield(L, -2, "sha3_256sum");

    lua_pushcfunction(L, lua_sha3_512sum);
    lua_setfield(L, -2, "sha3_512sum");

    lua_pushcfunction(L, lua_sha256sum);
    lua_setfield(L, -2, "sha256sum");

    lua_pushcfunction(L, lua_sha512sum);
    lua_setfield(L, -2, "sha512sum");

    lua_pushcfunction(L, lua_sleep);
    lua_setfield(L, -2, "sleep");

    lua_pushcfunction(L, lua_split);
    lua_setfield(L, -2, "split");

    lua_pushcfunction(L, lua_symlinkattr);
    lua_setfield(L, -2, "symlinkattr");

    lua_pushcfunction(L, lua_touch);
    lua_setfield(L, -2, "touch");

    // Lie la fonction createFileIterator à la table sous le nom "createFileIterator"
    lua_pushcfunction(L, lua_createFileIterator);
    lua_setfield(L, -2, "createFileIterator");

    // Enregistre la table globale "luapilot" dans l'état Lua
    lua_setglobal(L, "luapilot");

    luaopen_file_iterator(L);
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
