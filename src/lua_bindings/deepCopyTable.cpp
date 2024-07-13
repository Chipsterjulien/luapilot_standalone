#include "deepCopyTable.hpp"

// Fonction pour copier une table
void deepCopyTable(lua_State *L, int srcIndex, int destIndex, int &nextIndex) {
    lua_pushnil(L); // Première clé
    while (lua_next(L, srcIndex) != 0) {
        if (lua_type(L, -2) == LUA_TNUMBER) {
            // Clé numérique, ajouter à la fin de la liste
            if (lua_istable(L, -1)) {
                lua_newtable(L); // créer une nouvelle table pour la sous-table
                deepCopyTable(L, lua_gettop(L) - 1, lua_gettop(L), nextIndex);
                lua_rawseti(L, destIndex, nextIndex++); // Ajouter la nouvelle table au résultat
            } else {
                lua_pushvalue(L, -1);                   // Copier la valeur
                lua_rawseti(L, destIndex, nextIndex++); // Ajouter à la fin de la table de destination
            }
        } else {
            // Clé non numérique, copier normalement
            lua_pushvalue(L, -2); // Copier la clé
            if (lua_istable(L, -1)) {
                lua_newtable(L); // créer une nouvelle table pour la sous-table
                deepCopyTable(L, lua_gettop(L) - 1, lua_gettop(L), nextIndex);
                lua_settable(L, destIndex); // Insérer la nouvelle table dans le résultat
            } else {
                lua_pushvalue(L, -2);       // Copier la valeur
                lua_settable(L, destIndex); // Insérer dans la table de destination
            }
        }
        lua_pop(L, 1); // Supprimer la valeur, garder la clé pour lua_next
    }
}

// Interface pour la copie d'une table
int lua_deepCopyTable(lua_State *L) {
    // Vérifier qu'on a un argument en paramètre
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Vérifier que l'argument est une table
    if (!lua_istable(L, 1)) {
        return luaL_error(L, "Expected a table as argument");
    }

    // Créer une nouvelle table pour le résultat
    lua_newtable(L);

    // Copier la table source dans la table de destination
    int nextIndex = 1;
    deepCopyTable(L, 1, lua_gettop(L), nextIndex);

    // La nouvelle table copiée est maintenant en haut de la pile
    return 1;
}
