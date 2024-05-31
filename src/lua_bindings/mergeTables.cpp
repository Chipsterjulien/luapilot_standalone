#include "lua_bindings/mergeTables.hpp"

int lua_mergeTables(lua_State *L)
{
    // Vérifiez qu'il y a au moins deux arguments et que ce sont des tables
    int n = lua_gettop(L);
    if (n < 2)
    {
        return luaL_error(L, "Expected at least two tables as arguments");
    }

    for (int i = 1; i <= n; ++i)
    {
        if (!lua_istable(L, i))
        {
            return luaL_error(L, "Expected all arguments to be tables");
        }
    }

    // Créez une nouvelle table pour le résultat
    lua_newtable(L);
    int resultTableIndex = lua_gettop(L);

    // Fonction pour copier les éléments d'une table source à une table destination
    auto copyTable = [](lua_State *L, int srcIndex, int destIndex, int &nextIndex)
    {
        lua_pushnil(L); // Première clé
        while (lua_next(L, srcIndex) != 0)
        {
            if (lua_type(L, -2) == LUA_TNUMBER)
            {
                // Clé numérique, ajouter à la fin de la liste
                lua_pushvalue(L, -1);                   // Copier la valeur
                lua_rawseti(L, destIndex, nextIndex++); // Ajouter à la fin de la table de destination
            }
            else
            {
                // Clé non numérique, copier normalement
                lua_pushvalue(L, -2);       // Copier la clé
                lua_pushvalue(L, -2);       // Copier la valeur
                lua_settable(L, destIndex); // Insérer dans la table de destination
            }
            lua_pop(L, 1); // Supprimer la valeur, garder la clé pour lua_next
        }
    };

    int nextIndex = 1; // Indice pour les clés numériques
    // Copier chaque table passée en argument dans la nouvelle table
    for (int i = 1; i <= n; ++i)
    {
        lua_pushvalue(L, i);                                      // Copie de la table courante
        copyTable(L, lua_gettop(L), resultTableIndex, nextIndex); // Copie les éléments de la table au sommet de la pile vers la table en cours de construction
        lua_pop(L, 1);                                            // Supprime la copie de la table de la pile
    }

    // La nouvelle table fusionnée est maintenant en haut de la pile
    return 1; // Retourner la nouvelle table
}
