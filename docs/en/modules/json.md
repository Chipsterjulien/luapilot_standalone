> **English** | [Français](../../fr/modules/json.md)

# `babet.json` — JSON encode/decode

Backed by [nlohmann/json](https://github.com/nlohmann/json), the
most widely-used C++ JSON library. Handles the usual round-trip
plus the explicit "empty array" sentinel that Lua tables can't
disambiguate from "empty object".

## Why

The Lua/JSON impedance mismatch always trips someone up : Lua has
no distinction between an empty list and an empty table. Most
ad-hoc Lua JSON libraries pick one and get it wrong half the time.
`babet.json` is explicit : `{}` decodes to `{}` and re-encodes
to `{}` (object), but if you want `[]` you use the
`babet.json.empty_array` sentinel.

## API

| Function | Returns |
| --- | --- |
| `babet.json.encode(value, opts?)` | `string` (JSON text) \| `(nil, err)` |
| `babet.json.decode(text)` | `value` \| `(nil, err)` |
| `babet.json.empty_array` | sentinel value that encodes to `[]` |

Encoding options (`opts` table) :

- `pretty = true` — pretty-print with indentation (default `false`).
- `indent = N` — indent width when `pretty=true` (default `2`).

## Quick example

```lua
local J = babet.json

-- Decode
local data, err = J.decode([[
{ "name": "alice", "tags": ["admin", "ops"], "active": true }
]])
print(data.name, data.tags[1])

-- Encode (compact)
local out = J.encode({ a = 1, b = { 2, 3 } })
-- out == '{"a":1,"b":[2,3]}'

-- Encode pretty
local pretty = J.encode({ a = 1 }, { pretty = true, indent = 4 })

-- Empty array vs empty object
J.encode({})                    --> "{}"
J.encode(J.empty_array)         --> "[]"
J.encode({ items = J.empty_array })  --> '{"items":[]}'
```

## Error contract

- **`encode`** : `(nil, err)` on encoding errors (cycle in the
  graph, value that doesn't map to JSON like a function or
  userdata, NaN/Inf numbers, etc.).
- **`decode`** : `(nil, err)` on parse error. The error message
  includes the line/column where parsing failed.
- **Wrong argument types** → raises via `luaL_error`.

## Design decisions

- **`empty_array` sentinel** instead of guessing from the table
  structure (some libraries treat `{1, 2, 3}` as array and
  `{a=1}` as object, which is correct, but `{}` is ambiguous).
  An explicit sentinel removes all surprise.
- **NaN/Inf rejected**, not silently encoded as `null`. Different
  consumers handle JSON `null` differently for numeric fields ;
  better to fail loudly and let the caller decide.
- **No streaming API**. Round-trips are bounded by your script's
  memory regardless ; if you need to handle JSON larger than RAM,
  reach for a real streaming parser in a separate process.

## Not in v1

- Streaming encoder/decoder.
- A schema validator (use a pure-Lua one — they're trivial to
  add at the script level).
