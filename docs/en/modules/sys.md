> **English** | [Français](../../fr/modules/sys.md)

# `luapilot` sys — system utilities

Process and host introspection helpers : PID, hostname, kernel
info, executable lookup, environment variables.

## Why

Lua's standard library has `os.getenv` and `os.execute`, but
nothing for the PID, hostname, kernel info, or a portable `which`.
These are universal scripting needs that don't deserve a roundabout
via `popen` calls.

These functions live directly on `luapilot` (flat, no sub-table)
because they predate the per-module convention used by recent
additions.

## API

| Function | Returns |
| --- | --- |
| `luapilot.pid()` | `integer` — current process ID |
| `luapilot.hostname()` | `string` \| `(nil, err)` — system hostname |
| `luapilot.uname()` | `table` with `sysname`, `nodename`, `release`, `version`, `machine` |
| `luapilot.which(cmd)` | `string` (absolute path) \| `(nil, "not found")` |
| `luapilot.env(name)` | `string` \| `nil` — like `os.getenv`, but consistent |
| `luapilot.setenv(name, value)` | `(true, nil)` \| `(nil, err)` |

## Quick example

```lua
print("Running on:", luapilot.hostname(), "PID:", luapilot.pid())

local u = luapilot.uname()
print("Kernel:", u.sysname, u.release, u.machine)

-- Find a binary
local sh, err = luapilot.which("bash")
if sh then
    luapilot.exec({ sh, "-c", "echo hello" })
end

-- Read/write env
print("HOME:", luapilot.env("HOME"))
luapilot.setenv("MY_VAR", "value")
```

## Error contract

- **`pid()`** : never fails, always returns an integer.
- **`hostname()`** / **`which()`** : `(value, nil)` on success,
  `(nil, err_string)` on failure.
- **`env(name)`** : `value` if set, `nil` if unset. Never raises.
- **`setenv(name, value)`** : `(true, nil)` on success,
  `(nil, err)` on failure (rare — usually OOM or invalid name).
- **`uname()`** : never fails on any supported system, always
  returns the full table.
- **Wrong argument types** (e.g. `which(42)`) → raises via
  `luaL_error` after string coercion as per Lua convention. Pass a
  table or boolean to force a real error.

## Design decisions

- **`env(name)` returns `nil`, not `""`, for unset variables**.
  Matches `os.getenv` semantics. Tests should use
  `env("X") == nil`, not `env("X") == ""`.
- **`which` returns the absolute path**, not just "yes/no". This
  is more useful : you can `exec` the returned path directly. For
  a boolean check, use `which(cmd) ~= nil`.
- **`uname()` returns a table, not multiple values**. Makes it
  forward-compatible if a field is ever added (e.g. `domainname`).

## Not in v1

Additive — could be added later :

- `unsetenv` (just `setenv(name, nil)` could work too).
- `getuid` / `getgid` / `getppid` / `getlogin`.
- Full environment dump as a table.

For user lookups (UID → name, etc.) see the dedicated
[`user`](user.md) module — that uses NSS, which is the right
backend for that question.
