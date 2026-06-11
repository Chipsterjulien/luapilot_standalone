> **English** | [Français](../../fr/modules/time.md)

# `luapilot.time` — monotonic and realtime clocks

Two clocks with sub-second precision and a sleep that honours both
`s`/`ms`/`us`/`ns` units and signals. Fills the gaps in `os.time` /
`os.clock`.

## Why

Lua's `os.time()` only gives seconds, and `os.clock()` measures
CPU time, not wall time. Neither is appropriate for measuring real
durations (request latency, retry backoff, timeouts). And `os` has
no portable nanosecond sleep.

`luapilot.time` exposes `CLOCK_MONOTONIC` (durations) and
`CLOCK_REALTIME` (timestamps), with `sleep()` honouring signal
interruption.

## API

| Function | Returns |
| --- | --- |
| `luapilot.time.monotonic()` | `number` — seconds since arbitrary epoch (immune to clock changes) |
| `luapilot.time.realtime()` | `number` — POSIX time (seconds since 1970-01-01 UTC) |
| `luapilot.sleep(amount, unit?)` | `(true, nil)` \| `(nil, "interrupted")` |

`unit` for `sleep` is one of `"s"` (default), `"ms"`, `"us"`,
`"ns"`. Floats are accepted.

## Quick example

```lua
-- Measure a duration safely (immune to NTP / DST jumps)
local t0 = luapilot.time.monotonic()
do_something()
local elapsed = luapilot.time.monotonic() - t0
print(string.format("took %.3f s", elapsed))

-- Timestamp an event
local now = luapilot.time.realtime()
db:exec("INSERT INTO events (ts, kind) VALUES (?, ?)", { now, "ping" })

-- Sleep 100 ms, but wake up on a signal
local ok, err = luapilot.sleep(100, "ms")
if not ok and err == "interrupted" then
    print("woken by a signal")
end
```

## Error contract

- **`monotonic()` / `realtime()`** : never fail. Always return a
  number.
- **`sleep(amount, unit)`** :
  - `(true, nil)` if the sleep completed normally.
  - `(nil, "interrupted")` if a handled signal arrived during the
    sleep (see [`signal`](signal.md)).
- **NaN / Inf / negative amount** → raises via `luaL_error`.
- **Unknown unit string** → raises via `luaL_error`.

## Design decisions

- **Two clocks, two functions, no flag**. `monotonic()` for
  durations, `realtime()` for timestamps. Conflating them in one
  function with a parameter would invite the wrong choice.
- **Signal-aware `sleep`**. A 60-second sleep that ignores
  `SIGTERM` is the classic graceful-shutdown bug. `sleep()`
  returns `(nil, "interrupted")` so the caller can exit cleanly.
- **No "format date" helper**. `os.date(format, realtime())`
  works fine, and a duplicate function would just be a wrapper.

## Not in v1

- `luapilot.time.boottime()` (`CLOCK_BOOTTIME` — includes
  suspend). Rarely needed, easy to add later.
- `luapilot.time.utcnow()` / `localnow()` helpers that return
  pre-formatted strings. Trivial in Lua, no value-add.
