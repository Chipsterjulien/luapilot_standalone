#include "bundled_modules.hpp"

// Headers générés par tools/embed_lua_module.sh dans build/generated/.
// CMake ajoute ce dossier aux include paths.
#include "embedded_inspect.hpp"

namespace
{

    /**
     * @brief Loader function appelée par require("inspect") la première fois.
     *
     * Lua appelle ce code-ci automatiquement quand un script demande
     * require("inspect"), parce que la fonction est enregistrée dans
     * package.preload["inspect"]. Le résultat (la table retournée par
     * inspect.lua) est mis en cache par Lua : les require() suivants ne
     * relancent pas ce code, ils retournent directement la table cachée.
     */
    int load_bundled_inspect(lua_State *L)
    {
        if (luaL_loadbuffer(L, INSPECT_LUA_SOURCE,
                            sizeof(INSPECT_LUA_SOURCE) - 1,
                            "inspect") != LUA_OK)
        {
            return lua_error(L);
        }
        // Lua sait passer le nom du module au chunk (cf. require) ; on relaie.
        lua_pushvalue(L, 1);
        lua_call(L, 1, 1);
        return 1;
    }

} // namespace

void register_bundled_modules(lua_State *L)
{
    lua_getglobal(L, "package");    // [package]
    lua_getfield(L, -1, "preload"); // [package, preload]

    lua_pushcfunction(L, load_bundled_inspect);
    lua_setfield(L, -2, "inspect"); // preload["inspect"] = load_bundled_inspect

    lua_pop(L, 2); // [package, preload] → []
}
