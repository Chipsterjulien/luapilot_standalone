> **English** | [Français](../../fr/modules/strings.md)

# `luapilot` strings — string manipulation

A single function : split a string by a separator into a table.
Predates the sub-namespace convention, lives directly on
`luapilot`.

## Why

Splitting a string by a delimiter is one of the most common
string operations, and Lua's standard library doesn't have one
out of the box (`string.gmatch` works but requires pattern
escaping). A dedicated function is more discoverable and avoids
the pattern-escaping trap.

## API

| Function | Returns |
| --- | --- |
| `luapilot.split(s, sep)` | `table` (array) of substrings |

- `s` : the string to split.
- `sep` : the separator string (literal, no patterns).

If `sep` doesn't appear in `s`, the result is a one-element table
containing the whole `s`. If `s` is empty, returns an empty table.

## Quick example

```lua
local parts = luapilot.split("a,b,c,d", ",")
-- parts == { "a", "b", "c", "d" }

local one = luapilot.split("hello", ",")
-- one == { "hello" }
```

## Error contract

- **Wrong argument types** → raises via `luaL_error`.
- Otherwise always succeeds.

## Design decisions

- **Literal separator, not Lua pattern**. The common case is
  splitting CSV-like data, where you want `.` to mean a literal
  dot, not "any character". If you need pattern matching, fall
  back to `string.gmatch`.
- **Empty entries are preserved**. `"a,,b"` splits into
  `{"a", "", "b"}`. Consumers who want to filter empty strings
  do it in one line of Lua.

## Not in v1

- Pattern-based splitting (with escape rules). Use `string.gmatch`
  if needed.
- `joinTable(t, sep)` mirror — `table.concat` already exists in the
  stdlib.
