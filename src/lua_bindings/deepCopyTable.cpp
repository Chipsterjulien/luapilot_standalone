#include "deepCopyTable.hpp"
#include <unordered_map>
#include <cassert>

#define LUA_CHECK_STACK(L, n) assert(lua_gettop(L) >= (n))

/**
 * @brief Recursively deep copies a Lua table.
 *
 * @param L The Lua state.
 * @param srcIndex Index of the source table.
 * @param destIndex Index of the destination table.
 * @param nextNumericKey The next numeric key to use in the destination table.
 * @param depth Current depth of recursion.
 * @param maxDepth Maximum allowed depth for recursion.
 * @param visited Tables already visited to detect cycles.
 * @return True if successful, false otherwise.
 */
bool deepCopyTable(lua_State *L, int srcIndex, int destIndex, int &nextNumericKey, int depth, int maxDepth, VisitedMap& visited) {
    LUA_CHECK_STACK(L, 2);

    if (depth > maxDepth) {
        return false;
    }

    const void* srcPointer = lua_topointer(L, srcIndex);
    if (visited.find(srcPointer) != visited.end()) {
        lua_pushvalue(L, srcIndex);
        lua_settable(L, destIndex);
        return true;
    }

    visited[srcPointer] = true;

    lua_pushnil(L); // First key
    while (lua_next(L, srcIndex) != 0) {
        int valueIndex = lua_gettop(L);
        int keyIndex = valueIndex - 1;

        if (lua_type(L, keyIndex) != LUA_TNUMBER) {
            lua_pushvalue(L, keyIndex);
            if (lua_istable(L, valueIndex)) {
                lua_newtable(L);
                if (!deepCopyTable(L, valueIndex, lua_gettop(L), nextNumericKey, depth + 1, maxDepth, visited)) {
                    return false;
                }
                lua_settable(L, destIndex);
            } else {
                lua_pushvalue(L, valueIndex);
                lua_settable(L, destIndex);
            }
        }
        lua_pop(L, 1);
    }

    for (int i = 1;; ++i) {
        lua_rawgeti(L, srcIndex, i);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            break;
        }

        if (lua_istable(L, -1)) {
            lua_newtable(L);
            if (!deepCopyTable(L, lua_gettop(L) - 1, lua_gettop(L), nextNumericKey, depth + 1, maxDepth, visited)) {
                return false;
            }
            lua_rawseti(L, destIndex, nextNumericKey++);
        } else {
            lua_pushvalue(L, -1);
            lua_rawseti(L, destIndex, nextNumericKey++);
        }

        lua_pop(L, 1);
    }

    if (lua_getmetatable(L, srcIndex)) {
        lua_setmetatable(L, destIndex);
    }

    return true;
}

/**
 * @brief Lua binding for deep copying a Lua table.
 *
 * This function can be called from Lua with a table as the only argument.
 * It returns a deep copy of the input table.
 *
 * Lua usage:
 *     copiedTable = deepCopyTable(originalTable)
 *
 * @param L The Lua state.
 * @return int The number of values returned to Lua (1 in this case: the copied table).
 */
int lua_deepCopyTable(lua_State *L) {
    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Expected one argument (a table)");
    }

    if (!lua_istable(L, 1)) {
        return luaL_error(L, "Argument must be a table");
    }

    lua_newtable(L);

    int nextNumericKey = 1;
    VisitedMap visited;
    if (!deepCopyTable(L, 1, lua_gettop(L), nextNumericKey, 0, MAX_DEPTH, visited)) {
        return luaL_error(L, "Table is too deep to copy");
    }

    return 1;
}
