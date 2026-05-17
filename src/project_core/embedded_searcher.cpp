#include "embedded_searcher.hpp"
#include "zip_utils.hpp"
#include <algorithm>
#include <string>
#include <optional>
#include <vector>

static int embedded_lua_searcher(lua_State *L)
{
    const char *moduleName = luaL_checkstring(L, 1);
    const char *exePath = luaL_checkstring(L, lua_upvalueindex(1));

    // "foo.bar" -> "foo/bar"
    std::string base = moduleName;
    std::replace(base.begin(), base.end(), '.', '/');

    // On tente, dans l'ordre, les deux conventions Lua standard :
    //   1. foo/bar.lua
    //   2. foo/bar/init.lua
    // C'est la même logique que package.path en mode dossier, pour que le
    // comportement soit identique entre exécution depuis un dossier et
    // exécution depuis un zip embarqué.
    const std::string candidates[] = {
        base + ".lua",
        base + "/init.lua",
    };

    std::string tried; // accumule les chemins testés pour le message d'erreur

    for (const auto &path : candidates)
    {
        auto data = readEmbeddedFile(exePath, path);
        if (data)
        {
            if (luaL_loadbuffer(L, data->data(), data->size(), path.c_str()) != LUA_OK)
            {
                return lua_error(L);
            }
            // On retourne le loader (chunk compilé) + le chemin (info pour package).
            lua_pushstring(L, path.c_str());
            return 2;
        }
        tried += "\n\tno embedded file '" + path + "'";
    }

    // Aucun candidat trouvé : on push le message agrégé. Lua le concatènera
    // aux messages des autres searchers si tous échouent.
    lua_pushstring(L, tried.c_str());
    return 1;
}

void register_embedded_searcher(lua_State *L, const char *exePath)
{
    lua_getglobal(L, "package");      // [package]
    lua_getfield(L, -1, "searchers"); // [package, searchers]

    int len = static_cast<int>(lua_rawlen(L, -1));

    // Décale les searchers existants d'un cran à partir de l'index 2,
    // pour libérer la place 2 (après package.preload qui reste en 1).
    for (int i = len; i >= 2; --i)
    {
        lua_rawgeti(L, -1, i);
        lua_rawseti(L, -2, i + 1);
    }

    lua_pushstring(L, exePath);
    lua_pushcclosure(L, embedded_lua_searcher, 1); // upvalue = exePath
    lua_rawseti(L, -2, 2);

    lua_pop(L, 2); // [package, searchers] → []
}
