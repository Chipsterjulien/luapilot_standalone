> **English** | [Français](../../fr/modules/toml.md)

# `luapilot.toml` — TOML parser

Backed by [toml++](https://marzer.github.io/tomlplusplus/), a
strict TOML 1.0 parser. Decode only — encoding TOML faithfully
from a Lua table is ambiguous (Lua doesn't distinguish between
inline tables and dotted tables, for instance).

## Why

TOML is the natural config-file format for modern tools (Cargo,
pyproject, systemd-credentials, etc.). A LuaPilot script reading
its own config should be able to do it directly, not via a TOML →
JSON conversion via `tomlq`.

## API

| Function | Returns |
| --- | --- |
| `luapilot.toml.decode(text)` | `table` (parsed TOML) \| `(nil, err)` |

The returned table mirrors the TOML structure :

- Strings → Lua strings (UTF-8)
- Integers → Lua integers
- Floats → Lua numbers
- Booleans → Lua booleans
- Datetimes → Lua strings in ISO 8601 form (TOML has no
  Lua-native equivalent ; use `os.date` or a date library if you
  need parsed components)
- Arrays → Lua tables (1-indexed)
- Tables (`[section]` or `{ inline }`) → Lua tables (with string
  keys)

## Quick example

```lua
local body = [[
title = "My App"
[server]
host = "localhost"
port = 8080
features = ["auth", "metrics"]

[database]
url = "sqlite:///var/lib/app.db"
]]

local cfg, err = luapilot.toml.decode(body)
if not cfg then error("config parse failed: " .. err) end

print(cfg.title)                  -- "My App"
print(cfg.server.host)            -- "localhost"
print(cfg.server.features[1])     -- "auth"
print(cfg.database.url)           -- "sqlite:///var/lib/app.db"
```

## Error contract

- **`decode`** : `(nil, err)` on parse error. The error message
  includes the line/column where parsing failed, and a short
  description of what was wrong (per toml++).
- **Non-string argument** → raises via `luaL_error`.

## Design decisions

- **Decode only, no encode**. Round-tripping a TOML file via a
  Lua table loses formatting (comments, dotted vs inline tables,
  newlines between sections) that real users care about. If you
  need to write TOML, build the string yourself — TOML is
  designed to be human-writable. A faithful encoder would be a
  large undertaking for limited value.
- **Datetimes as strings**. TOML's datetime type is rich (offset,
  local, date-only, time-only) but Lua has no native equivalent.
  Returning a string in ISO 8601 form preserves the data losslessly
  and lets the script parse it with the tools it prefers.

## Not in v1

- Encoder. May be added if a real use case appears, but unlikely.
- A schema validator. Build one in Lua if you need it ; the
  decoded structure is just a table.
