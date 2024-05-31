#include "lua_bindings/tableCopy.hpp"

// Fonction pour copier une table Lua (superficielle ou profonde)
void copyTable(lua_State *L, int srcIndex, int destIndex, bool deep)
{
    lua_pushnil(L); // Première clé
    while (lua_next(L, srcIndex) != 0)
    {
        // Copier la clé
        lua_pushvalue(L, -2);

        // Copier la valeur
        if (deep && lua_istable(L, -1))
        {
            // Si copie profonde et valeur est une table, copier récursivement
            lua_newtable(L);
            copyTable(L, lua_gettop(L) - 1, lua_gettop(L), true);
        }
        else
        {
            lua_pushvalue(L, -1);
        }

        // Insérer dans la table de destination
        lua_settable(L, destIndex);

        // Supprimer la valeur, garder la clé pour lua_next
        lua_pop(L, 1);
    }
}

// Wrapper pour la copie superficielle
int lua_shallowCopy(lua_State *L)
{
    // Vérifier qu'il y a au moins un argument et que c'est une table
    if (!lua_istable(L, 1))
    {
        return luaL_error(L, "Expected a table as the first argument");
    }

    // Créer une nouvelle table pour le résultat
    lua_newtable(L);

    // Copier la table source dans la table de destination
    copyTable(L, 1, lua_gettop(L), false);

    // La nouvelle table copiée est maintenant en haut de la pile
    return 1; // Retourner la nouvelle table
}

// Wrapper pour la copie profonde
int lua_deepCopy(lua_State *L)
{
    // Vérifier qu'il y a au moins un argument et que c'est une table
    if (!lua_istable(L, 1))
    {
        return luaL_error(L, "Expected a table as the first argument");
    }

    // Créer une nouvelle table pour le résultat
    lua_newtable(L);

    // Copier la table source dans la table de destination
    copyTable(L, 1, lua_gettop(L), true);

    // La nouvelle table copiée est maintenant en haut de la pile
    return 1; // Retourner la nouvelle table
}
