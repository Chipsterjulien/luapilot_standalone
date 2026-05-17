#ifndef EXEC_HPP
#define EXEC_HPP

#include <lua.hpp>

/**
 * @brief Lua binding: runs an external program and captures its output.
 *
 * Lua usage:
 *   result, err = luapilot.exec(cmd [, args] [, opts])
 *     cmd  : string  — program to run (PATH is searched)
 *     args : table   — array of string arguments (optional)
 *     opts : table   — { cwd = string, env = { KEY = VALUE, ... } } (optional)
 *
 *   On successful launch:
 *     result = { stdout = string, stderr = string, code = integer }
 *     err    = nil
 *   On launch failure (program not found, cwd invalid, ...):
 *     result = nil
 *     err    = string
 *
 *   A program that runs but exits non-zero is NOT an error here — that is
 *   reported via result.code. The env table is merged with the current
 *   environment, it does not replace it.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack (2).
 */
int lua_exec(lua_State *L);

#endif // EXEC_HPP