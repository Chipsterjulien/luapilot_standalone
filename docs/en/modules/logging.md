> **English** | [Français](../../fr/modules/logging.md)

# `logging` — leveled logger (Lua module)

A standard leveled logger : `debug`, `info`, `warn`, `error`,
`fatal`. Bundled as a pure-Lua module, loaded via
`require("logging")`. Not in the `babet` namespace — it's a
library module, like `inspect`.

## Why

Every non-trivial script ends up reinventing some flavour of
"levels + timestamps + optional file output". Standardising it
removes the bikeshed and makes log output consistent across
Babet scripts.

## API

```lua
local log = require("logging")
```

| Function | Returns |
| --- | --- |
| `log.set_level(name)` | `(true, nil)` — `name` is one of `"debug"`, `"info"`, `"warn"`, `"error"`, `"fatal"` |
| `log.set_output(path)` | `(true, nil)` \| `(nil, err)` — file path (default stderr) |
| `log.debug(...)`, `log.info(...)`, `log.warn(...)`, `log.error(...)`, `log.fatal(...)` | each prints if its level is ≥ current threshold |

Messages are timestamped (`YYYY-MM-DD HH:MM:SS`) and tagged with
the level (`[DEBUG]`, `[INFO]`, etc.). Multiple arguments are
concatenated with spaces, like `print`.

## Quick example

```lua
local log = require("logging")
log.set_level("info")   -- debug calls below this become no-ops

log.info("startup, pid =", babet.pid())
log.warn("retry attempt", 3, "of", 5)
log.error("connection failed:", err)

-- Optional: route to a file
local ok, err = log.set_output("/var/log/myapp.log")
if not ok then
    log.fatal("can't open log:", err)
    os.exit(1)
end
```

Output :

```
2026-06-11 14:32:01 [INFO] startup, pid = 12345
2026-06-11 14:32:05 [WARN] retry attempt 3 of 5
2026-06-11 14:32:05 [ERROR] connection failed: timeout
```

## Error contract

- **`set_level(name)`** : unknown name raises via `error()`.
- **`set_output(path)`** : `(nil, err)` if the file can't be
  opened for append. Logging continues to the previous sink
  (stderr if none was set).
- Logging functions themselves never fail — if the file becomes
  unwritable mid-run, the message is silently dropped (the
  alternative — crashing the script because the log volume is
  full — is worse).

## Design decisions

- **Pure-Lua module**. No C++ needed, and being in Lua means
  scripts can monkey-patch it (e.g. add JSON-format output) at
  the call site without rebuilding Babet.
- **Five levels, no `trace`**. Five is enough for almost everyone
  ; adding more invites everyone to invent their own.
- **Single global sink**. Multiple loggers / hierarchical loggers
  / per-module levels are a feature creep trap. If you need
  multiple destinations, write a wrapper.

## Not in v1

- JSON / structured logging output. Easy to bolt on at the
  script level if needed.
- Log rotation. Use `logrotate(8)` on the file output.
- Async / buffered output. Premature optimisation for typical use.
