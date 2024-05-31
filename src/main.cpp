#include "lua_bindings/fileExists.hpp"
#include "lua_bindings/fileSize.hpp"
#include "lua_bindings/fileUtils.hpp"
#include "lua_bindings/helloThere.hpp"
#include "lua_bindings/listFiles.hpp"
#include "lua_bindings/memoryUtils.hpp"
#include "lua_bindings/split.hpp"
#include "lua_bindings/tableCopy.hpp"
#include "project_core/loadLuaFile.hpp"
#include "project_core/zip_utils.hpp"
#include "project_core/create_executable.hpp"
#include "project_core/help.hpp"
#include <iostream>
#include <lua.hpp>
#include <filesystem>
#include <physfs.h>

namespace fs = std::filesystem;

void register_luapilot(lua_State *L)
{
    lua_newtable(L);

    lua_pushcfunction(L, lua_fileExists);
    lua_setfield(L, -2, "fileExists");

    lua_pushcfunction(L, lua_deepCopy);
    lua_setfield(L, -2, "deepCopy");

    lua_pushcfunction(L, lua_fileSize);
    lua_setfield(L, -2, "fileSize");

    lua_pushcfunction(L, lua_getBasename);
    lua_setfield(L, -2, "getBasename");

    lua_pushcfunction(L, lua_getExtension);
    lua_setfield(L, -2, "getExtension");

    lua_pushcfunction(L, lua_getFilename);
    lua_setfield(L, -2, "getFilename");

    lua_pushcfunction(L, lua_getPath);
    lua_setfield(L, -2, "getPath");

    lua_pushcfunction(L, lua_helloThere);
    lua_setfield(L, -2, "helloThere");

    lua_pushcfunction(L, lua_listFiles);
    lua_setfield(L, -2, "listFiles");

    lua_pushcfunction(L, lua_getMemoryUsage);
    lua_setfield(L, -2, "getMemoryUsage");

    lua_pushcfunction(L, lua_shallowCopy);
    lua_setfield(L, -2, "shallowCopy");

    lua_pushcfunction(L, split);
    lua_setfield(L, -2, "split");

    lua_setglobal(L, "luapilot");
}

int main(int argc, char *argv[])
{
    // Vérifier si une option d'aide est présente
    for (int i = 1; i < argc; ++i)
    {
        std::string option = argv[i];
        if (option == "--help" || option == "-h")
        {
            printHelp();
            return 0;
        }
    }

    if (argc < 2)
    {
        // Initialiser PhysFS
        if (!initPhysFS(argv[0]))
        {
            return 1;
        }

        // Nom de l'archive ZIP à monter
        const char *archive = "embedded.zip";

        // Chemin du fichier main.lua dans l'archive montée
        std::string mainLuaPath = "main.lua";

        // Monter l'archive ZIP intégrée
        if (!mountZipFile(argv[0]))
        {
            return 1;
        }

        if (!PHYSFS_exists(mainLuaPath.c_str()))
        {
            std::cerr << "Erreur : main.lua introuvable dans l'archive ZIP" << std::endl;
            PHYSFS_deinit();
            return 1;
        }

        // Exécuter le script Lua
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);

        register_luapilot(L);

        char *fileData;
        PHYSFS_File *file = PHYSFS_openRead(mainLuaPath.c_str());
        PHYSFS_sint64 fileSize = PHYSFS_fileLength(file);
        fileData = new char[fileSize];
        PHYSFS_readBytes(file, fileData, fileSize);
        PHYSFS_close(file);

        if (luaL_loadbuffer(L, fileData, fileSize, mainLuaPath.c_str()) || lua_pcall(L, 0, LUA_MULTRET, 0))
        {
            std::cerr << "Erreur : " << lua_tostring(L, -1) << std::endl;
            delete[] fileData;
            lua_close(L);
            PHYSFS_deinit();
            return 1;
        }

        delete[] fileData;
        lua_close(L);
        PHYSFS_deinit();
        return 0;
    }

    std::string option = argv[1];

    if (option == "--create-exe" || option == "-c")
    {
        if (argc != 4)
        {
            std::cerr << "Usage : " << argv[0] << " --create-exe <dir> <output>" << std::endl;
            return 1;
        }
        std::string dir = argv[2];
        std::string output = argv[3];
        createExecutableWithDir(dir, output);
        return 0;
    }

    if (option == "--help" || option == "-h")
    {
        printHelp();
        return 0;
    }

    std::string path = argv[1];
    std::string mainLuaPath = fs::path(path) / "main.lua";

    if (!fs::exists(mainLuaPath))
    {
        std::cerr << "Erreur : main.lua introuvable dans le répertoire " << path << std::endl;
        return 1;
    }

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    register_luapilot(L);

    loadLuaFile(L, mainLuaPath);

    lua_close(L);
    return 0;
}
