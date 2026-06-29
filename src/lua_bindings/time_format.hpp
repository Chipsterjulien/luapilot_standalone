#ifndef TIME_FORMAT_HPP
#define TIME_FORMAT_HPP

#include <lua.hpp>

/**
 * Lua bindings exposed under the `babet.time` sub-table.
 *
 *   babet.time.iso(ts?)            -> "YYYY-MM-DDTHH:MM:SSZ"
 *   babet.time.parse_iso(s)        -> (unix_ts integer, nil)
 *                                     |  (nil, "parse_iso: ...")
 *   babet.time.parse_duration(s)   -> (seconds integer, nil)
 *                                     |  (nil, "parse_duration: ...")
 *   babet.time.format_duration(n)  -> "1d2h3m4s"
 *
 * All timestamps are interpreted as Unix time (seconds since
 * 1970-01-01 UTC). All formatting is UTC (suffix "Z"); no timezone
 * database, no locale, no calendar arithmetic.
 *
 * Error contract:
 *   - Pure-computation functions (iso, format_duration) raise via
 *     luaL_error for invalid argument types (caller bugs).
 *   - Parsers (parse_iso, parse_duration) report user-input failures
 *     as (nil, "<funcname>: <reason>"), and raise via luaL_error only
 *     on type bugs (not a string).
 *   - Error messages are prefixed by the function name, matching the
 *     convention used by sqlite, user, crc32sum, etc.
 */
int lua_time_iso(lua_State *L);
int lua_time_parse_iso(lua_State *L);
int lua_time_parse_duration(lua_State *L);
int lua_time_format_duration(lua_State *L);

#endif // TIME_FORMAT_HPP
