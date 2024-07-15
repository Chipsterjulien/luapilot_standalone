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

    // Function to copy elements from a source table to a destination table
    auto copyTable = [](lua_State *L, int srcIndex, int destIndex, int &nextIndex) {
        lua_pushnil(L); // First key
        while (lua_next(L, srcIndex) != 0) {
            if (lua_type(L, -2) == LUA_TNUMBER) {
                // Numeric key, add to the end of the list
                lua_pushvalue(L, -1);                   // Copy value
                lua_rawseti(L, destIndex, nextIndex++); // Add to the end of the destination table
            } else {
                // Non-numeric key, copy normally
                lua_pushvalue(L, -2);       // Copy key
                lua_pushvalue(L, -2);       // Copy value
                lua_settable(L, destIndex); // Insert into the destination table
            }
            lua_pop(L, 1); // Remove value, keep key for lua_next
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
