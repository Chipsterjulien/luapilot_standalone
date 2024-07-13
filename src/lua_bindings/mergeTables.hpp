#ifndef MERGE_TABLES_HPP
#define MERGE_TABLES_HPP

#include <lua.hpp>

/**
 * Lua binding for merging multiple tables.
 * @param L The Lua state.
 * @return Number of return values (2: merged table or nil, and error message or nil).
 * Lua usage: mergedTable, err = lua_mergeTables(table1, table2, ...)
 */
int lua_mergeTables(lua_State *L);

#endif // MERGE_TABLES_HPP
