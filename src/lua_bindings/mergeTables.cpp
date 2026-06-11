#include "mergeTables.hpp"

/**
 * Lua binding for merging multiple tables.
 * @param L The Lua state.
 * @return Number of return values (1: merged table).
 * Lua usage: mergedTable = lua_mergeTables(table1, table2, ...)
 */
int lua_mergeTables(lua_State* L) {
    // Verify that there are at least two arguments and that they are tables
    int n = lua_gettop(L);
    if (n < 2) {
        return luaL_error(L, "Expected at least two tables as arguments");
    }

    for (int i = 1; i <= n; ++i) {
        if (!lua_istable(L, i)) {
            return luaL_error(L, "Expected all arguments to be tables");
        }
    }

    // Create a new table for the result
    lua_newtable(L);
    int resultTableIndex = lua_gettop(L);

    // Function to copy elements from a source table to a destination table.
    //
    // Two-pass strategy :
    //   1) Partie array : itération ordonnée 1..n via lua_rawgeti, pour
    //      garantir que mergeTables({1,2}, {3,4}) donne {1,2,3,4} et
    //      pas une permutation. lua_next sur la partie array d'une
    //      table ne garantit PAS l'ordre.
    //   2) Partie hash : lua_next pour les clés non-array (string ou
    //      number hors de [1, n]). Les clés integer dans [1, n] sont
    //      déjà traitées par la passe 1, on les saute.
    auto copyTable = [](lua_State *L, int srcIndex, int destIndex, int &nextIndex) {
        // --- Passe 1 : partie array, ordonnée ---
        lua_Integer n = luaL_len(L, srcIndex);
        for (lua_Integer i = 1; i <= n; ++i)
        {
            lua_rawgeti(L, srcIndex, i);             // pousse t[i]
            lua_rawseti(L, destIndex, nextIndex++);  // dest[next] = t[i], pop
        }

        // --- Passe 2 : partie hash ---
        lua_pushnil(L); // first key
        while (lua_next(L, srcIndex) != 0)
        {
            // Pile : ..., key, value
            // On saute les clés integer dans [1, n] (déjà copiées
            // par la passe 1).
            if (lua_type(L, -2) == LUA_TNUMBER && lua_isinteger(L, -2))
            {
                lua_Integer k = lua_tointeger(L, -2);
                if (k >= 1 && k <= n)
                {
                    lua_pop(L, 1); // pop value, keep key for lua_next
                    continue;
                }
            }
            // Clé non-array (string, float, integer hors range) :
            // on copie key+value et on settable. lua_settable
            // pop key+value, donc on dup les deux avant.
            lua_pushvalue(L, -2);       // dup key
            lua_pushvalue(L, -2);       // dup value
            lua_settable(L, destIndex); // dest[key] = value, pop 2
            lua_pop(L, 1);              // pop original value, keep key
        }
    };

    int nextIndex = 1; // Index for numeric keys
    // Copy each table passed as argument into the new table
    for (int i = 1; i <= n; ++i) {
        lua_pushvalue(L, i);                                      // Copy current table
        copyTable(L, lua_gettop(L), resultTableIndex, nextIndex); // Copy elements from the table at the top of the stack to the table being constructed
        lua_pop(L, 1);                                            // Remove the copy of the table from the stack
    }

    // The new merged table is now at the top of the stack
    return 1; // Return merged table
}
