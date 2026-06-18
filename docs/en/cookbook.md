> **English** | [Français](../fr/cookbook.md)

# Cookbook

Concrete recipes for common scripting tasks. Each recipe is short
and self-contained ; copy, adapt, ship.

## Watch a directory and email new files

A drop directory that mails out every file as it arrives. Uses
`luapilot.inotify` for instant wakeup (no polling), and listens on
`close_write` + `moved_to` so the file is guaranteed complete.

```lua
local w = assert(luapilot.inotify.new())
assert(w:add("/srv/incoming", { "close_write", "moved_to" }))

luapilot.signal.handle("TERM", function()
    w:close()
    os.exit(0)
end)

while true do
    local events, err = w:read()
    if not events then
        if err == "interrupted" then break end
        io.stderr:write("inotify: ", err, "\n"); break
    end
    for _, ev in ipairs(events) do
        if ev.events.overflow then
            -- queue overflowed: rescan the directory
            -- (events were lost)
        else
            local path = "/srv/incoming/" .. ev.name
            send_email(path)
            os.remove(path)
        end
    end
end
```

## Ensure a service user exists, then drop to it

A typical install-time check before launching a daemon as a
non-root user.

```lua
local function ensure_user(name)
    if luapilot.user.exists(name) then return true end
    local ok, err = luapilot.exec("useradd", {
        "--system",
        "--home-dir", "/var/lib/" .. name,
        "--create-home",
        "--shell", "/usr/sbin/nologin",
        name,
    })
    if not ok then
        return nil, "useradd failed: " .. tostring(err)
    end
    return true
end

assert(ensure_user("myapp"))
local u = assert(luapilot.user.get("myapp"))
luapilot.chdir(u.home)
```

## Fetch many URLs in parallel

`luapilot.workers` runs Lua on multiple cores. Total wall-clock is
close to the slowest single fetch, not the sum.

```lua
local urls = { "https://a.example/", "https://b.example/",
               "https://c.example/", "https://d.example/" }

local jobs = {}
for i, url in ipairs(urls) do
    jobs[i] = luapilot.workers.spawn([[
        local url = worker.args.url
        local r, err = luapilot.http.get(url, { timeout = 10 })
        if not r then return { error = err } end
        return { status = r.status, length = #r.body }
    ]], { url = url })
end

for i, job in ipairs(jobs) do
    local ok, result = job:join()
    print(i, ok, result and result.status or result.error)
end
```

## Graceful shutdown of a long-running script

Catch `SIGTERM` / `SIGINT` so the script can close resources
properly. Works with `systemctl stop` and Ctrl-C.

```lua
local log = require("logging")
log.set_level("info")

local running = true
luapilot.signal.handle("TERM", function()
    log.info("SIGTERM, shutting down")
    running = false
end)
luapilot.signal.handle("INT", function()
    log.info("Ctrl-C, shutting down")
    running = false
end)

while running do
    -- main loop ; blocking calls return (nil, "interrupted") when
    -- a handled signal arrives, so the loop exits quickly.
    local line, err = sock:recv_line()
    if not line then
        if err == "interrupted" then break end
        log.error("recv: ", err); break
    end
    process(line)
end

sock:close()
log.info("clean shutdown")
```

## Read a TOML config, validate, persist to SQLite

```lua
-- Load TOML
local f = assert(io.open("config.toml", "r"))
local body = f:read("a"); f:close()
local cfg, perr = luapilot.toml.decode(body)
if not cfg then error("config.toml: " .. perr) end

-- Validate (cheap manual checks ; for a real schema, build one in Lua)
assert(type(cfg.server) == "table", "missing [server]")
assert(type(cfg.server.host) == "string", "missing server.host")
assert(math.type(cfg.server.port) == "integer", "server.port must be integer")

-- Open db, persist
local db = assert(luapilot.sqlite.open("state.db", { wal = true }))
db:exec([[CREATE TABLE IF NOT EXISTS config (k TEXT PRIMARY KEY, v TEXT)]])
db:exec("INSERT OR REPLACE INTO config VALUES (?, ?)",
        { "host", cfg.server.host })
db:exec("INSERT OR REPLACE INTO config VALUES (?, ?)",
        { "port", tostring(cfg.server.port) })
db:close()
```

## Cache with a human-readable TTL, expiry as ISO timestamps

Parse a TTL from config (`"15m"`, `"2h30m"`, `"1d"`), compute a
deadline, and log it in a uniform UTC format that grep'ing across
log files actually works on.

```lua
-- Config from TOML, CLI, env, anywhere.
local raw_ttl = "15m"

local ttl, err = luapilot.time.parse_duration(raw_ttl)
if not ttl then error("bad TTL '" .. raw_ttl .. "': " .. err) end

local deadline = luapilot.now() + ttl
print(string.format("[%s] cache entry created, expires at %s (TTL %s)",
    luapilot.time.iso(),
    luapilot.time.iso(deadline),
    luapilot.time.format_duration(ttl)))

-- ...later, somewhere else in the script:
if luapilot.now() >= deadline then
    print("[" .. luapilot.time.iso() .. "] cache miss: entry expired")
end
```

Useful properties of this pattern :

- `luapilot.time.iso()` is always UTC, always 20 characters, always
  the same shape. `grep` and `sort` work on the timestamps directly.
- `parse_duration("15m")` returns an integer, so `now() + ttl` is
  exact (no float drift over long durations).
- `format_duration(ttl)` round-trips : `parse_duration(
  format_duration(n)) == n` for every `n >= 0`. Safe to use as a
  canonical representation in logs or databases.

## Pretty-print a Lua value

The bundled `inspect` module gives readable output for nested
tables.

```lua
local inspect = require("inspect")
print(inspect({ a = 1, b = { c = 2, d = { 3, 4, 5 } } }))
-- { a = 1, b = { c = 2, d = { 3, 4, 5 } } }
```

## Hello there

The minimal LuaPilot script :

```lua
luapilot.helloThere()
```
