# luaPilot

LuaPilot is a binary that provides advanced functionalities such as splitting strings, merging tables, listing files with or without an iterator, computing checksums, and more. It is written in C++23 for optimal performance and allows embedding a ZIP file into the executable if you want. When the executable runs, embedded files (main.lua and any `require`d module) are read on demand directly from the appended ZIP.

It is currently compiled to work with Lua 5.5.0, but you can change the version by editing `build_local.sh` and recompiling.

See the **[examples](https://github.com/Chipsterjulien/luapilot_standalone/tree/main/examples)** if you want to learn more …

## Download the latest version

To download the latest version of LuaPilot, visit the [releases page on GitHub](https://github.com/Chipsterjulien/luapilot_standalone/releases) and download the most recent release.

## Compilation

If you want to compile the project, do this:

1. **Clone the repository:**

```sh
git clone https://github.com/Chipsterjulien/luapilot_standalone.git
cd luapilot_standalone
```

2. **Build the project locally:**

```sh
./build_local.sh
```

The build script downloads and compiles its own dependencies (Lua, OpenSSL, miniz), so the only requirements on your system are a C++23 compiler, CMake, wget and unzip.

## Usage

The entry point is `main.lua`. You can load other Lua files from it with `require`.

```sh
./luapilot .
```

If a ZIP is embedded:

```sh
./luapilot_with_zip_embedded
```

### To create an executable from a Lua script using the --create-exe option

If luapilot is installed globally:

```sh
# for example:
echo 'luapilot.helloThere()' > main.lua
luapilot --create-exe . luapilot_with_lua_script_embedded
./luapilot_with_lua_script_embedded
```

If luapilot is not installed globally:

```sh
# for example:
echo 'luapilot.helloThere()' > main.lua
./luapilot --create-exe . luapilot_with_lua_script_embedded
./luapilot_with_lua_script_embedded
```

In this example, LuaPilot creates a ZIP file from the current directory since `.` is the first argument.

If you want to use your own ZIP:

```sh
cat luapilot my_file.zip > my_new_exec_program
chmod +x my_new_exec_program
./my_new_exec_program
```

## Security model

**LuaPilot runs trusted Lua scripts. It is not a sandbox.**

Scripts execute with the full Lua standard library (`luaL_openlibs`),
and LuaPilot additionally exposes filesystem and process primitives such
as `exec`, `remove`, `rmdirAll` and `chdir`. A script therefore has the
same privileges as the process running it: it can read, modify or delete
files and run arbitrary commands. This is by design — like `make`, a
shell script, or any build tool, LuaPilot is meant to run code you
control.

Only run `main.lua` and `require`d modules that you trust. Do **not**
use LuaPilot to execute Lua from untrusted sources; it provides no
isolation against hostile code, and none is intended. If you need to run
untrusted scripts, use a purpose-built Lua sandbox instead.

The hardening work in LuaPilot (path/symlink handling, process-group
cleanup, output limits, dependency checksums) protects *legitimate* use
against accidents and supply-chain tampering. It does not turn LuaPilot
into a barrier against malicious scripts, and should not be relied on as
one.

## Some examples

### Hello there

```lua
luapilot.helloThere()
```

### File Manipulation

```lua
-- List files in a directory
local files, err = luapilot.listFiles(".")
if err then
    print(err)
else
    for _, file in ipairs(files) do
        print(file)
    end
end

-- Check if a file exists
local exists, err = luapilot.fileExists("my/folder/file.txt")
if err then
    print(err)
else
    print("File exists: " .. tostring(exists))
end
```

### Table manipulations

```lua
local inspect = require('inspect')
-- Merge two tables
local table1 = {a = 1, b = 2}
local table2 = {b = 3, c = 4}
local mergedTable = luapilot.mergeTables(table1, table2)
print(inspect(mergedTable))

-- Merge multiple tables
local mergedMultipleTables = luapilot.mergeTables(table1, table2, {d = 5}, {e = 6})
print(inspect(mergedMultipleTables))

-- Make a deep copy of a table
local t4 = { i = 8, j = { k = 9, l = 10, m = { n = 11, o = 12 } } }
local deepCopy = luapilot.deepCopyTable(t4)
print(inspect(deepCopy))
```

### String manipulation

```lua
-- Split a string
local parts = luapilot.split("hello,world", ",")
for _, part in ipairs(parts) do
    print(part)
end
```

### JSON

```lua
-- Encode (compact, or pretty with opts.indent)
local s, err = luapilot.json.encode({ name = "ada", tags = { "x", "y" } })
local pretty = luapilot.json.encode({ a = 1 }, { indent = 2 })

-- Decode
local value, derr = luapilot.json.decode('{"n": 42, "ok": true}')

-- Sentinels :
--   luapilot.json.null         <-> JSON null (distinct de nil :
--                                  aller-retour sans perte de clé)
--   luapilot.json.empty_array  -> force [] (une table vide {} donne {})
local doc = { items = luapilot.json.empty_array, missing = luapilot.json.null }

-- as_array : force une table à être sérialisée comme array, même vide.
-- Utile pour un tableau construit dynamiquement qui peut finir vide.
local tags = {}
-- ... remplissage conditionnel de tags ...
local s2 = luapilot.json.encode({ tags = luapilot.json.as_array(tags) })
-- tags resté vide -> {"tags":[]} (et non {"tags":{}})
```

`as_array(t)` renvoie `t` (chaînable) et le marque via sa métatable —
elle écrase donc une métatable préexistante sur `t` (sans conséquence
pour des tables de données, le seul cas où l'on sérialise du JSON).

Une table aux clés `1..n` devient un tableau JSON ; une table à clés
string devient un objet. Les clés mixtes, les tableaux à trous, NaN /
Infinity et les valeurs non encodables renvoient `(nil, err)` — jamais
d'exception Lua.

See the [examples](https://github.com/Chipsterjulien/luapilot_standalone/tree/main/examples) if you want to learn more …

### HTTP client

`luapilot.http` is an HTTP/HTTPS client. HTTPS reuses the OpenSSL
already linked into LuaPilot — no extra TLS dependency.

Its error model mirrors `luapilot.exec`:

* a **transport failure** (DNS, connection refused, TLS, timeout, …)
  returns `(nil, "http: <reason>")`. The reason string is the raw
  cpp-httplib error label — for example `"http: Timeout"` on a global
  timeout, `"http: Connection"` on a refused connection. No temporal
  heuristic is applied to rename it;
* a **completed exchange returns `(result, nil)` whatever the status**
  — a 404 or 500 is *not* an error, exactly like a non-zero exit code
  is not an error for `exec`. The status lives in `result.status`;
* **misuse** (missing `url`, wrong option *type*) raises a Lua error,
  like other LuaPilot bindings. A bad option *value* (unknown method,
  malformed URL, `timeout <= 0`, body on GET/HEAD/OPTIONS) returns
  `(nil, err)` — no silent behavior.

```lua
-- Simple GET
local res, err = luapilot.http.get("https://example.com")
if not res then
    print("request failed: " .. err)        -- transport failure
else
    print(res.status)                       -- e.g. 200, 404, 500
    print(res.body)                          -- binary-safe (NUL ok)
    print(res.headers["content-type"])       -- header keys lowercased
end

-- POST with a body
local r = luapilot.http.post("https://api.example.com/items",
    '{"name":"ada"}',
    { headers = { ["Content-Type"] = "application/json" } })

-- Generic form
local r2, e2 = luapilot.http.request({
    url     = "https://example.com/search",
    method  = "GET",
    query   = { q = "lua http", page = 2 },  -- percent-encoded
    headers = { Accept = "text/html" },
    timeout = 5,                             -- seconds (end-to-end)
})
```

**`request(opts)` options**

| Field              | Type   | Default      | Meaning                                                                                                         |
| ------------------ | ------ | ------------ | --------------------------------------------------------------------------------------------------------------- |
| `url`              | string | — (required) | `http://` or `https://`                                                                                         |
| `method`           | string | `"GET"`      | GET/HEAD/OPTIONS/POST/PUT/PATCH/DELETE                                                                          |
| `headers`          | table  | `{}`         | `{ ["Name"] = "value" }`                                                                                        |
| `body`             | string | —            | request body (body methods only; supplying it on GET/HEAD/OPTIONS returns `(nil, err)`)                         |
| `query`            | table  | —            | `{ k = v }` → `?k=v&…`, percent-encoded                                                                         |
| `timeout`          | number | none         | seconds (> 0); end-to-end max time, equivalent to curl `--max-time` (applied via cpp-httplib `set_max_timeout`) |
| `verify`           | bool   | `true`       | TLS server certificate verification                                                                             |
| `ca_cert`          | string | —            | path to a CA bundle                                                                                             |
| `follow_redirects` | bool   | `false`      | not followed unless asked explicitly                                                                            |

`get(url [, opts])` and `post(url [, body] [, opts])` are thin wrappers
over `request` (single code path).

**`result` table**

| Field     | Type    | Notes                                                                                      |
| --------- | ------- | ------------------------------------------------------------------------------------------ |
| `status`  | integer | HTTP status (any value, incl. 4xx/5xx)                                                     |
| `body`    | string  | binary-safe (embedded NUL bytes preserved)                                                 |
| `headers` | table   | keys **lowercased** (HTTP is case-insensitive, Lua is not); on duplicates, last value wins |

Not in v1 (addable later under SemVer without breakage): streaming,
multipart, cookies, sessions, exposed keep-alive. There is **no HTTP
server** in LuaPilot's API.

See the [examples](https://github.com/Chipsterjulien/luapilot_standalone/tree/main/examples) if you want to learn more …

### argparse — command-line argument parser

`argparse` is a pure-Lua module bundled in the LuaPilot binary, loaded
via `require("argparse")`. It builds a parser, parses `arg` (or an
explicit table), and returns a single `(res, err)` contract — no
implicit `print`, no `os.exit`. The script always stays in control.

```lua
local argparse = require("argparse")

local p = argparse("myprog", "do something useful")
    :flag("-v --verbose",     { help = "verbose output" })
    :option("-o --output",    { default = "a.out", help = "output file" })
    :option("-m --mode",      { choices = { "fast", "safe" } })
    :argument("input",        { help = "input file" })

local res, err = p:parse()       -- reads `arg[1..n]` by default
if not res then
    io.stderr:write(err, "\n")   -- bad input from the user
    os.exit(2)
elseif res.help then
    io.write(res.usage)          -- -h / --help is a success
    os.exit(0)
end

-- normal case: res.input, res.output, res.verbose, res.mode
```

**Three return shapes from `parse()`**, all under the same `(res, err)`
contract:

| Case            | `res`                               | `err`                                                                 |
| --------------- | ----------------------------------- | --------------------------------------------------------------------- |
| Success         | `{ dest = value, … }`               | `nil`                                                                 |
| `-h` / `--help` | `{ help = true, usage = "<text>" }` | `nil` (help is a **success**, never an error)                         |
| Bad user input  | `nil`                               | `"unknown option '--nope'"`, `"missing required argument 'input'"`, … |

`parse()` **never raises a Lua error on user input** — every runtime
failure is reported via `(nil, err)`. The only errors raised are
*programmer* mistakes in the builder (empty spec, option name without
`-`, duplicate name), analogous to a `luaL_error` from a C binding.

**Builder methods** (each returns `self`, so they chain):

| Method                          | Purpose                                         |
| ------------------------------- | ----------------------------------------------- |
| `p:flag("-v --verbose", opts)`  | boolean flag, defaults to `false`               |
| `p:option("-o --output", opts)` | option that takes one value                     |
| `p:argument("name", opts)`      | positional argument                             |
| `p:parse([t])`                  | parse `arg[1..n]` by default, or table `t`      |
| `p:get_usage()`                 | help text (also exposed as `res.usage` on `-h`) |

**Option spec**: whitespace-separated short and long names
(`"-o --output"`), at least one must start with `-`. The destination
name is taken from the first long form (`--output` → `res.output`),
falling back to the first short form; `opts.dest` overrides explicitly.

**`opts` table** (all fields optional):

| Field      | Applies to       | Meaning                                                                                                        |
| ---------- | ---------------- | -------------------------------------------------------------------------------------------------------------- |
| `default`  | option, argument | value when absent; a positional with `default` becomes optional                                                |
| `required` | option, argument | force presence (positional is required by default unless `default` is set)                                     |
| `choices`  | option, argument | list of allowed values, tested on the **raw** string                                                           |
| `convert`  | option, argument | function `(raw) -> value` applied **after** `choices`; returning `nil` or raising is reported via `(nil, err)` |
| `help`     | all              | text used in `get_usage()` / `res.usage`                                                                       |
| `dest`     | option           | override the result key                                                                                        |

> **Note on `choices` and `convert`**: `choices` is checked on the raw
> string before `convert` runs. If you combine both, express `choices`
> as strings — e.g. `choices = {"1", "2", "3"}` with
> `convert = tonumber` — not as the converted values.

**Argument source.** `parse()` without an argument reads the global
`arg` at indices `1..n` (the actual user arguments, identical across
all three LuaPilot launch modes — packaged, folder, embedded-via-PATH).
Indices `<= 0` (binary path, folder name) are intentionally ignored.
You can pass an explicit table: `p:parse({"-v", "input.txt"})`.

**Supported argument forms**: `--long value`, `--long=value`,
`-s value`, `-s=value`. A `--` token terminates option parsing — every
token after it is treated as positional (so `-h` after `--` is the
literal string `"-h"`, not the help flag).

**Not in v1** (addable later, pure-Lua additive — no SemVer breakage):
subcommands, variadic `nargs`, glued short forms (`-abc`, `-fvalue`).

See the [examples](https://github.com/Chipsterjulien/luapilot_standalone/tree/main/examples) if you want to learn more …

### logging — leveled logger

`logging` is a pure-Lua module bundled in the LuaPilot binary, loaded
via `require("logging")`. It provides leveled logging with timestamps,
a configurable output sink, and optional ANSI colors — but stays
small, predictable, and side-effect-free unless you ask for output.

```lua
local log = require("logging")

log.info("user=", uid, " connected")           -- variadic like print
log.warn("slow query (", elapsed_ms, "ms)")
log.error("connection refused:", host)

log.set_level("debug")                          -- "trace"/"debug"/... or log.DEBUG
log.set_output(io.open("app.log", "a"))         -- any object with :write
log.set_color(true)                             -- ANSI OFF by default, opt-in
```

**Output format** (fixed in v1, no custom formatter yet):

```
2026-05-22 14:32:01 [INFO ] user= 42  connected
```

The level label is padded to 5 characters so columns stay aligned
(`[TRACE]`, `[DEBUG]`, `[INFO ]`, `[WARN ]`, `[ERROR]`).

**Levels** (`>=` threshold passes, others are silently dropped — the
only legitimate silence, since you asked for it):

| Name                 | Constant    | Numeric |
| -------------------- | ----------- | ------- |
| trace                | `log.TRACE` | 10      |
| debug                | `log.DEBUG` | 20      |
| **info** *(default)* | `log.INFO`  | 30      |
| warn                 | `log.WARN`  | 40      |
| error                | `log.ERROR` | 50      |

**API**

| Function                                                                                   | Purpose                                                                                           |
| ------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------- |
| `log.trace(...)` / `log.debug(...)` / `log.info(...)` / `log.warn(...)` / `log.error(...)` | emit a message; variadic args are converted with `tostring` and joined with spaces (like `print`) |
| `log.set_level(level)`                                                                     | accepts a string (`"info"`, case-insensitive) or a number (`log.INFO`)                            |
| `log.set_output(sink)`                                                                     | any object responding to `:write` (default `io.stderr`)                                           |
| `log.set_color(on)`                                                                        | strict boolean; default `false`                                                                   |
| `log.get_level()` / `log.get_output()` / `log.get_color()`                                 | introspection                                                                                     |

**Defaults**

- Level: **`info`** (so `trace` and `debug` are dropped until you ask for them)
- Output: **`io.stderr`** (Unix convention — keeps `print()` and other
  stdout data uncontaminated by diagnostics)
- Color: **`false`** (explicit opt-in; no auto-detection in v1)

**Colors** (when `set_color(true)`):

| Level    | Color                                                          |
| -------- | -------------------------------------------------------------- |
| trace    | dim (gray)                                                     |
| debug    | cyan                                                           |
| **info** | none (neutral — info is the default flow, no attention needed) |
| warn     | yellow                                                         |
| error    | red                                                            |

**Error contract**

* The five emission functions (`log.trace` … `log.error`) **never raise
  and never return anything**. If `:write` fails (full disk, closed
  handle, sink that throws), the failure is silently swallowed. This
  is the standard logger rule: a missed log line must not turn into a
  business-logic crash.
* The setters (`set_level`, `set_output`, `set_color`) **raise via
  `error()` on misuse** (unknown level name, non-writable output,
  non-boolean color) — these are programmer mistakes, analogous to a
  C binding's `luaL_error`.

**Not in v1** (all addable later, pure-Lua additive — no SemVer
breakage): file rotation, named loggers, logger hierarchy and
propagation, custom filters, custom format strings, milliseconds or
UTC in timestamps, automatic TTY detection (`isatty` opt-in).

See the [examples](https://github.com/Chipsterjulien/luapilot_standalone/tree/main/examples) if you want to learn more …

### sys — system utilities

`luapilot.sys` exposes a small set of system-level utilities, but
attached **flat under `luapilot.*`** (no nested `sys` subtable) — they
sit next to `exec`, `sleep`, `currentDir`, etc. The list is short and
these are scripting basics, so the flat layout keeps things simple.

```lua
print(luapilot.hostname())                       -- "my-laptop"
print(luapilot.pid())                            -- 12345

local sh = luapilot.which("sh")                  -- "/usr/bin/sh"
local missing, err = luapilot.which("nope_42")   -- nil, "which: 'nope_42' not found in PATH"

local port = luapilot.env("PORT") or "8080"      -- env-or-default idiom

local ok_set, err = luapilot.setenv("LANG", "C")
if not ok_set then io.stderr:write(err, "\n") end

local u = luapilot.uname()                       -- { sysname, nodename, release, version, machine }
print(u.sysname, u.machine)                      -- "Linux", "x86_64"
```

**Functions**

| Function              | Returns                       | Notes                                                                                                                                         |
| --------------------- | ----------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------- |
| `which(name)`         | `"path"` \| `(nil, err)`      | Searches `$PATH` for an executable. If `name` contains `/`, the path is tested directly (mirrors `/usr/bin/which`). Returns an absolute path. |
| `env(name)`           | `"value"` \| `nil`            | Reads an environment variable. **Absence is `nil` alone** (no error message), allowing the `or default` idiom. Wrong argument type raises.    |
| `setenv(name, value)` | `(true, nil)` \| `(nil, err)` | Writes an environment variable (overwrite). Process-global side effect. Invalid `name` (e.g. containing `=`) returns `(nil, err)`.            |
| `hostname()`          | `"host"` \| `(nil, err)`      | Wraps `gethostname(2)`.                                                                                                                       |
| `uname()`             | table \| `(nil, err)`         | Wraps `uname(2)`. Table has `sysname`, `nodename`, `release`, `version`, `machine` — all non-empty strings (equivalent of `uname -a` fields). |
| `pid()`               | integer                       | Wraps `getpid(2)`, cannot fail (POSIX).                                                                                                       |

**Error contract** (consistent with the rest of LuaPilot):

* **Success** — the value is returned directly (no wrapping).
* **Expected runtime failure** (`which("nope")`, `setenv("bad=name", ...)`) — `(nil, "msg")`.
* **Programmer misuse** (missing argument, wrong type) — raises via `luaL_error`.
* **`env`** is the deliberate exception: an absent variable is **not** an error, it returns `nil` alone. This is for the `local p = luapilot.env("PORT") or "8080"` idiom, which is the dominant use of env reads.

> **Note on `env(name)` argument**: like every other LuaPilot binding,
> `env` uses `luaL_checkstring`, which silently coerces *numbers* to
> their string form (Lua convention). So `env(42)` is equivalent to
> `env("42")` and returns whatever variable `"42"` resolves to (likely
> `nil`). Pass a table or boolean to force a Lua error.

**Not in v1** (all addable later, additive — no SemVer breakage):
`unsetenv`, `getuid`/`getgid`, `getppid`, `getlogin`, full
environment dump.

See the [examples](https://github.com/Chipsterjulien/luapilot_standalone/tree/main/examples) if you want to learn more …

### TOML

`luapilot.toml.decode` parses a TOML string into a Lua table. The
binding wraps [toml++](https://github.com/marzer/tomlplusplus) (v1.0.0
of the TOML spec, header-only) and mirrors the `luapilot.json` error
contract.

```lua
local toml = luapilot.toml

local cfg, err = toml.decode([[
title = "My App"

[server]
host = "127.0.0.1"
port = 8080

[[users]]
name = "alice"
age = 30

[[users]]
name = "bob"
age = 25
]])

if not cfg then
    io.stderr:write("toml: ", err, "\n")
    return
end

print(cfg.title)                  -- "My App"
print(cfg.server.host)            -- "127.0.0.1"
print(cfg.users[1].name)          -- "alice"
```

**Type mapping** (TOML → Lua):

| TOML type        | Lua type                   | Notes                                                     |
| ---------------- | -------------------------- | --------------------------------------------------------- |
| string           | string                     | UTF-8 preserved                                           |
| integer          | number (integer)           | 64-bit, Lua 5.5 integer subtype                           |
| float            | number (float)             | IEEE 754 double                                           |
| boolean          | boolean                    |                                                           |
| array            | table (1-indexed sequence) | mirrors `json.decode` behavior                            |
| table            | table (string keys)        | including dotted and `[section]`/`[[section]]` forms      |
| local-date       | string                     | ISO 8601: `"1979-05-27"`                                  |
| local-time       | string                     | ISO 8601: `"07:32:00"` (with optional fractional seconds) |
| local-date-time  | string                     | ISO 8601: `"1979-05-27T07:32:00"`                         |
| offset-date-time | string                     | ISO 8601: `"1979-05-27T07:32:00Z"` or `"...+02:00"`       |

> **On temporal types**: TOML has four distinct date/time types; Lua
> has no native date type. We render them as ISO 8601 strings — lossy
> (the original type tag is gone) but predictable. Re-parse with the
> tool of your choice if you need structured access.

**Error contract** (strict mirror of `json.decode`):

* **Success** — `(table, nil)`. A TOML document always has a table at
  the root; the returned value is therefore always a table (possibly
  empty for an empty document or a file with only comments).
* **Invalid TOML** — `(nil, "toml: <description> (line L, col C)")`.
  Includes redefined keys, unknown literals (`TRUE` is not valid TOML
  — only lowercase `true`/`false`), unterminated strings, etc.
* **Programmer misuse** (missing argument, non-string type) — raises
  via `luaL_error`. Unlike most LuaPilot string-accepting functions,
  `toml.decode` uses `luaL_checktype(LUA_TSTRING)` strictly: passing
  a number raises rather than coercing.

```lua
-- Wrong: tries to feed a number to the TOML parser
local _, err = toml.decode(42)   -- error: bad argument
```

**Not in v1** (all addable later under SemVer):

* No `encode` — TOML's structured-table-to-section mapping has many
  defensible answers; we'd rather not bake a wrong default. Use
  `json.encode` for machine-to-machine round-trips.
* No `decode_file(path)` — use `local s = assert(io.open(path)):read("a"); toml.decode(s)`.
* No sentinels (TOML has no `null`).
* No parsing options — TOML is a strict format by design.

See the [examples](https://github.com/Chipsterjulien/luapilot_standalone/tree/main/examples) if you want to learn more …

### sockets — TCP client and server

`luapilot.socket` provides blocking TCP sockets with timeouts — both
client (`connect`) and server (`listen`/`accept`) sides. Pure POSIX
under the hood (zero dependency), userdata objects with `__gc` so a
forgotten `close()` never leaks a file descriptor.

```lua
local socket = luapilot.socket

-- Client side
local cli, err = socket.connect("example.com", 80, 5)   -- 5s connect timeout
if not cli then
    io.stderr:write(err, "\n"); return
end
cli:set_timeout(2)                                       -- 2s per operation after
cli:send("GET / HTTP/1.0\r\nHost: example.com\r\n\r\n")
local response = cli:recv_all()
cli:close()

-- Server side (echoes one line back, then closes)
local srv = assert(socket.listen("127.0.0.1", 8080))
local peer = assert(srv:accept())
local line = peer:recv_line()
peer:send("you said: " .. line .. "\n")
peer:close()
srv:close()
```

**Functions**

| Function                                 | Returns                | Notes                                                                                                                                       |
| ---------------------------------------- | ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------- |
| `socket.connect(host, port [, timeout])` | socket \| `(nil, err)` | TCP client. `timeout` is in seconds (float) and applies to the connect phase only.                                                          |
| `socket.listen(host, port [, backlog])`  | socket \| `(nil, err)` | Does `socket + setsockopt(SO_REUSEADDR) + bind + listen` in one go. `host = ""` binds all interfaces (`AI_PASSIVE`). Default backlog is 16. |

**Methods** (on a connected socket from `connect()` or `accept()`):

| Method             | Returns                                                                  | Notes                                                                                                                                                            |
| ------------------ | ------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `s:send(data)`     | `(n_bytes, nil)` \| `(nil, "closed")` \| `(nil, err)`                    | Sends the full `data` (loops on partial writes). Peer closed → `(nil, "closed")`. Uses `MSG_NOSIGNAL` so a closed peer never raises `SIGPIPE`.                   |
| `s:recv(n)`        | `(data, nil)` \| `(nil, "closed")` \| `(nil, "timeout")` \| `(nil, err)` | Reads **up to** `n` bytes (POSIX `read()` semantics — may return less). EOF is the typed string `"closed"`.                                                      |
| `s:recv_line()`    | `(line, nil)` \| `(nil, "closed", partial)` \| `(nil, "timeout")`        | Reads up to `\n` exclusive. CRLF transparent (`\r` before `\n` is stripped). If EOF hits mid-line, returns **three values** — don't lose the bytes already read. |
| `s:recv_all()`     | `(data, nil)` \| `(nil, "timeout")` \| `(nil, err)`                      | Reads until peer EOF (the normal *success* condition for this one).                                                                                              |
| `s:set_timeout(s)` | `(true, nil)` \| `(nil, err)`                                            | Seconds (float). `0` = no timeout (block forever). Applies to all subsequent blocking operations on this socket.                                                 |
| `s:close()`        | `(true, nil)`                                                            | Idempotent (calling on an already-closed socket is fine).                                                                                                        |
| `s:peer()`         | `{host, port}` \| `(nil, err)`                                           | Remote address of the connection.                                                                                                                                |
| `s:sockname()`     | `{host, port}` \| `(nil, err)`                                           | Local address of the socket.                                                                                                                                     |

**Methods on a listening socket** (from `listen()`):

| Method                                            | Returns                                      | Notes                                                                                                         |
| ------------------------------------------------- | -------------------------------------------- | ------------------------------------------------------------------------------------------------------------- |
| `s:accept()`                                      | socket \| `(nil, "timeout")` \| `(nil, err)` | Accepts an incoming connection. Returns a new connected socket. Honors `set_timeout` on the listening socket. |
| `s:close()` / `s:set_timeout(s)` / `s:sockname()` | —                                            | Same as on connected sockets.                                                                                 |
| `s:send` / `s:recv*`                              | `(nil, err)`                                 | Always fails: wrong direction.                                                                                |

**Error contract**

* **Success** — value (socket, byte count, data, …) directly.
* **Expected runtime failures** are typed strings, returned as `(nil, str)`:
  * `"closed"` — peer closed the connection (graceful EOF). For `recv_line` mid-line, the partial bytes come as a 3ʳᵈ return value.
  * `"timeout"` — `set_timeout` expired.
  * Other transport failures get a `"socket: <description>"` prefix.
* **Programmer misuse** (missing argument, wrong type) — raises via `luaL_error`. Like the rest of LuaPilot, `host` and `port` go through `luaL_checkstring`/`luaL_checkinteger`, which silently coerce numbers and numeric strings. Pass a table to force an error.

**Internal `SO_REUSEADDR`**

`socket.listen` sets `SO_REUSEADDR` on the socket before `bind()`,
unconditionally. This is what lets a server you killed restart on the
same port without waiting ~60 seconds for `TIME_WAIT` to expire — the
default case for tests, dev servers, and small daemons. Not exposed in
the v1 API; we'd rather have the behavior right by default than make
you pass a flag every time.

**Not in v1** (additive later under SemVer):

* **UDP** — `socket.udp(...)` with the same shape, when needed.
* **Non-blocking mode** — `s:set_blocking(false)` plus a `select`/
  `poll` primitive. Heavily overlaps with the workers chantier;
  doing it standalone would mean shipping 30 % of workers under
  another name.
* **Advanced options** — `keepalive`, `nodelay`, `SO_REUSEPORT`,
  `SO_LINGER`, custom buffer sizes, etc. Added on demand.
* **`bind()` exposed separately** — `listen` does it all-in-one by
  design (decision SOCK-2). Exposing raw `bind` is possible but no
  use case yet requires it.

For TLS client sockets (`connect_tls` and `starttls`), see the next
section.

See the [examples](https://github.com/Chipsterjulien/luapilot_standalone/tree/main/examples) if you want to learn more …

### sockets TLS — connect_tls and starttls

`luapilot.socket` also provides TLS client sockets, sharing the same
userdata and methods (`send`, `recv`, `recv_line`, `recv_all`,
`set_timeout`, `close`, `peer`, `sockname`) as plain TCP. TLS is a
**variant** of TCP, not a separate module — once connected, the
socket behaves identically.

```lua
local socket = luapilot.socket

-- Direct TLS connection (IRC over TLS on Libera.Chat, etc.)
local s, err = socket.connect_tls("irc.libera.chat", 6697,
    { timeout = 5 })
if not s then io.stderr:write(err, "\n"); return end
s:set_timeout(30)
s:send("NICK my-bot\r\nUSER my-bot 0 * :LuaPilot bot\r\n")
while true do
    local line, e = s:recv_line()
    if not line then break end
    print(line)
end
s:close()

-- STARTTLS pattern (SMTP, IMAP, XMPP, IRC with CAP STARTTLS, etc.)
local plain, err = socket.connect("smtp.example.com", 587, 5)
plain:set_timeout(10)
plain:recv_line()                              -- read server greeting
plain:send("EHLO me.example.com\r\n")
-- (read EHLO response, send STARTTLS, read OK response...)
local ok, terr = plain:starttls({ hostname = "smtp.example.com" })
if not ok then io.stderr:write(terr, "\n"); return end
-- From here on, plain is now encrypted. Send the next EHLO over TLS.
plain:send("EHLO me.example.com\r\n")
```

**Functions and methods**

| Call                                      | Returns                       | Notes                                                                                                                                      |
| ----------------------------------------- | ----------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| `socket.connect_tls(host, port [, opts])` | socket \| `(nil, err)`        | TCP connect + TLS handshake in one call. Mirror of `connect`.                                                                              |
| `s:starttls([opts])`                      | `(true, nil)` \| `(nil, err)` | Promote an existing connected TCP socket to TLS in place. Used for protocols that negotiate TLS over plaintext (SMTP/IMAP STARTTLS, etc.). |

**Options table `opts`** (all fields optional):

| Field         | Type             | Default                    | Meaning                                                                                                                                                                                                                                                     |
| ------------- | ---------------- | -------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `timeout`     | number (seconds) | none                       | Per-call deadline. Applies to the connect phase and the TLS handshake. After the socket is established, use `s:set_timeout()` for ongoing reads/writes.                                                                                                     |
| `verify`      | boolean          | `true`                     | When true, the peer certificate is validated against the trust store **and** against the expected hostname. When false, the TLS handshake completes regardless of certificate validity — use only for debug, self-signed test servers, or explicit opt-out. |
| `ca_cert`     | string (path)    | none                       | Path to a PEM-encoded CA certificate file. Use when the peer is signed by a custom CA not in the system store.                                                                                                                                              |
| `ca_path`     | string (path)    | none                       | Path to a directory of PEM-encoded CA certificates (OpenSSL `c_rehash` format). Less common than `ca_cert`.                                                                                                                                                 |
| `hostname`    | string           | `host` (for `connect_tls`) | Hostname used for SNI and for the certificate's CN/SAN check. Override when connecting by IP but verifying a vhost (e.g., `connect_tls("1.2.3.4", 443, {hostname="api.example.com"})`).                                                                     |
| `min_version` | string           | `"1.2"`                    | Minimum TLS version. `"1.2"` or `"1.3"`. Negotiation always picks the highest mutually supported (TLS 1.3 if both sides agree).                                                                                                                             |

**Important: `starttls` and `verify`.** Unlike `connect_tls`, `starttls`
has no implicit hostname (the socket comes from a previous `connect`
which may have used an IP). When `verify=true` (default), you **must**
pass `opts.hostname` explicitly. The call refuses otherwise with
`tls: starttls with verify=true requires opts.hostname`. This avoids
the silent "chain-only" verification that some libraries default to,
which is weaker than it looks.

**Error contract** (consistent with plain sockets):

* **Success** — value (socket, byte count, etc.) directly.
* **Typed string errors** as `(nil, str)`:
  * `"closed"` — peer closed the TLS session (close_notify received).
  * `"timeout"` — per-call deadline expired.
* **Other errors** are prefixed `"tls: "`. Certificate validation
  failures are explicit: e.g., `tls: certificate verify failed:
  certificate has expired`, `tls: certificate verify failed:
  Hostname mismatch`, `tls: certificate verify failed: self-signed
  certificate`.
* **Programmer misuse** (wrong arg types, missing arguments) raises
  via `luaL_error` as elsewhere in LuaPilot. Pass a table to force
  an error on `host`/`port` (numeric strings are coerced silently —
  same convention as the rest of LuaPilot).

**Certificate verification policy.** Strict by default (`verify=true`):
the chain is validated against the system trust store, and the
certificate's CN/SAN must match `host` (or `opts.hostname` if
provided). To disable verification entirely, pass `verify=false`
explicitly — there is no middle ground (no "chain only, no hostname"
mode).

**System CA store.** OpenSSL's `SSL_CTX_set_default_verify_paths()` is
used, which on Linux means `/etc/ssl/certs/` (managed by the
`ca-certificates` package). No bundled CA list inside the LuaPilot
binary — system updates apply automatically. Override with `ca_cert` or
`ca_path` when needed.

**TLS versions.** TLS 1.2 minimum by default; TLS 1.3 is negotiated
automatically if both sides support it. Older protocols (SSL 3, TLS
1.0, TLS 1.1) are unconditionally rejected — they are broken and no
modern server should rely on them.

**Not in v1** (additive later under SemVer):

* **Server-side TLS** — `socket.listen_tls()` and the corresponding
  certificate/key loading. Reported because adding server TLS opens
  six or seven design decisions (PEM/DER, file path vs in-memory,
  passphrase handling, certificate hot-reload…) that deserve their
  own design pass.
* **Client certificates (mutual TLS)** — only server-side
  verification is supported today; clients cannot present their own
  certificate.
* **ALPN** — needed for HTTP/2 negotiation; not for IRC, SMTP, IMAP,
  XMPP, or other text protocols.
* **Session resumption** — a performance optimization, not a
  functional gap.
* **DTLS** — UDP-based TLS.

See the [examples](https://github.com/Chipsterjulien/luapilot_standalone/tree/main/examples) if you want to learn more …

### workers — parallel jobs

`luapilot.workers` runs Lua code in parallel using OS threads.
Each worker has its **own `lua_State`** and **its own memory** — no
shared state, no race conditions at the Lua level. Communication
happens through **serialized messages** (JSON internally). This is
the Erlang / Web Workers model: isolate, message, join.

Unlike languages with a global interpreter lock (Python GIL),
LuaPilot workers really run on multiple CPU cores. Spawn N workers
and you can use N cores.

The API supports **two usage patterns**:

* **Fork-join** — spawn a worker that computes one result and dies.
  Use `:join()` or `:poll()` to retrieve the return value.
* **Persistent** — spawn a worker that loops on `worker.recv()`,
  processes messages, and replies via `worker.send()`. Use the
  parent-side `w:send()` / `w:recv()` / `w:close()` to communicate.

Both patterns share the same `spawn()` call and the same
isolation model. The choice is in the worker's code: a `return`
statement at the top level is fork-join; a `while ... worker.recv()`
loop is persistent.

#### Fork-join: compute one result

```lua
local workers = luapilot.workers

-- Trivial: spawn a worker, wait for the result
local w = workers.spawn("return 1 + 2")
local ok, value = w:join()    -- ok=true, value=3

-- Pass arguments to the worker
local w2 = workers.spawn(
    "return worker.args.x * worker.args.y",
    { x = 6, y = 7 })
local ok, value = w2:join()   -- ok=true, value=42

-- Canonical pattern: fan out N CPU-bound jobs
local urls = { "http://a/", "http://b/", "http://c/", "http://d/" }
local jobs = {}
for i, url in ipairs(urls) do
    jobs[i] = workers.spawn([[
        local url = worker.args.url
        local r, err = luapilot.http.get(url, { timeout = 10 })
        if not r then return { error = err } end
        return { status = r.status, length = #r.body }
    ]], { url = url })
end
-- Join all (blocks until each finishes). Total wall-clock ≈ slowest
-- single fetch, not the sum.
for i, job in ipairs(jobs) do
    local ok, result = job:join()
    print(i, ok, result and result.status or result.error)
end
```

#### Persistent: bidirectional messaging

```lua
-- Worker that echoes every message it receives, with a marker.
local w = workers.spawn([[
    while true do
        local ok, msg = worker.recv()  -- blocks on parent input
        if not ok then break end       -- "closed": parent shutting down
        worker.send({ echo = msg })
    end
    return "exited cleanly"
]])

w:send("hello")
w:send("world")
w:send(42)

local _, r1 = w:recv()    -- (true, { echo = "hello" })
local _, r2 = w:recv()    -- (true, { echo = "world" })
local _, r3 = w:recv()    -- (true, { echo = 42 })

-- Signal end-of-input. The worker's worker.recv() returns
-- (false, "closed"), it exits the loop, the return value crosses
-- via join().
w:close()
local ok, value = w:join()    -- (true, "exited cleanly")
```

Typical use cases for persistent workers: a logger worker, a
database connection worker, a long-running protocol handler (IRC,
WebSocket), or anything that should outlive a single request.

#### API reference

| Call                                    | Returns                              | Notes                                                                                                                                                                                                                                                      |
| --------------------------------------- | ------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `workers.spawn(code [, args] [, opts])` | worker \| `(nil, err)`               | Start a worker. `opts.inbox_capacity` and `opts.outbox_capacity` set the bounded queue sizes (default 64 each). Returns `(nil, err)` if serialization fails (`function`, `userdata`, `coroutine`, cycle, NaN/Inf, non-UTF-8) or if `pthread_create` fails. |
| `w:join()`                              | `(ok, value)`                        | **Blocks** until the worker finishes. `ok=true, value=<return value>` on success; `ok=false, value=<error message>` if the worker raised.                                                                                                                  |
| `w:poll()`                              | `(state, value)`                     | **Non-blocking** worker-state check. `state` is `"running"`, `"done"`, or `"error"`. `value` is nil while running, the return value when done, the error message on error.                                                                                 |
| `w:send(value [, timeout])`             | `(true, nil)` \| `(false, reason)`   | Push a message to the worker's inbox. Blocks if full; `timeout` is in seconds. `reason` ∈ `"full"` (timeout=0 and queue full), `"timeout"` (timeout>0 expired), `"closed"`.                                                                                |
| `w:recv([timeout])`                     | `(true, value)` \| `(false, reason)` | Pop a message from the worker's outbox. Blocks if empty; `timeout` is in seconds. `reason` ∈ `"empty"` (timeout=0 and queue empty), `"timeout"`, `"closed"`.                                                                                               |
| `w:close()`                             | `(true, nil)`                        | Closes the worker's inbox. Subsequent `w:send` returns `(false, "closed")`; the worker's `worker.recv()` returns `(false, "closed")` so it can exit its loop cleanly. Idempotent.                                                                          |

Inside the worker:

| Call                             | Returns                              | Notes                                                                                     |
| -------------------------------- | ------------------------------------ | ----------------------------------------------------------------------------------------- |
| `worker.send(value [, timeout])` | `(true, nil)` \| `(false, reason)`   | Push a message to the parent (outbox direction). Blocks if outbox is full (backpressure). |
| `worker.recv([timeout])`         | `(true, value)` \| `(false, reason)` | Pop a message from the parent (inbox direction). Blocks if empty.                         |
| `worker.args`                    | table \| nil                         | The args table passed at `spawn()`, or nil.                                               |

#### Return convention conventions

The convention varies between **parent-side** and **worker-side**:

* **Parent-side `w:send` / `w:recv` / `w:join` / `w:close`** use the
  pcall-style `(ok, value_or_reason)` — `value` may legitimately be
  `nil` after a successful exchange, so `(false, "...")` is the only
  way to signal failure unambiguously.
* **Parent-side `w:send` with a non-serializable value** returns
  `(nil, err)` instead of `(false, err)` — this is the `(result,
  err)` convention used everywhere else in LuaPilot when a *bad
  value* is passed at the API boundary (compare with
  `luapilot.json.encode`).
* **Worker-side `worker.send` / `worker.recv`** consistently use
  `(ok, value_or_reason)` — no exception. Inside the worker, you
  always write `local ok, msg = worker.recv(); if not ok then ... end`.

#### Important: `w:close()` is non-interruptive

`w:close()` performs a **logical close** of the worker's inbox —
it does **not** interrupt the worker if it is currently blocked in
a long operation (a slow `socket:recv()`, a long sleep, a long
`http.get()`, etc.). The worker only sees the closure the *next
time* it calls `worker.recv()`.

```lua
-- ANTI-PATTERN: this worker is uninterruptible
local w = workers.spawn([[
    local sock = luapilot.socket.connect("slow-server", 80, 30)
    sock:set_timeout(60)
    sock:recv(4096)  -- blocks up to 60 seconds, ignoring w:close()
    return "done"
]])

w:close()  -- worker keeps reading on the socket; this only affects
           -- the inbox, which the worker is not reading from anyway
w:join()   -- waits up to 60 seconds
```

To make a worker shutdown-responsive, **write it cooperatively**:
interleave `worker.recv(0)` calls between long operations and exit
the loop when you see `(false, "closed")`:

```lua
-- COOPERATIVE PATTERN
local w = workers.spawn([[
    while true do
        -- Quick non-blocking check: parent wants us to stop?
        local ok, msg = worker.recv(0)
        if not ok and msg == "closed" then break end
        if ok then
            -- handle msg
        end

        -- Do one unit of work (short and bounded). Do NOT do an
        -- operation that could block longer than the desired
        -- shutdown latency.
        do_one_step()
    end
    return "exited cleanly"
]])
```

`:kill()` is intentionally **not** part of the v1 API: forcefully
terminating a worker mid-pthread leaves shared state (file
descriptors, mutexes, OpenSSL contexts) in unrecoverable shapes.
Cooperative shutdown is the only safe model. If a worker must be
able to interrupt a long socket operation, set `set_timeout()` to
the worst-case acceptable latency.

#### Argument and message serialization

Arguments passed at `spawn()` and messages exchanged via
`send`/`recv` must be JSON-representable:

| Lua type                                   | Supported? | Notes                                                                           |
| ------------------------------------------ | ---------- | ------------------------------------------------------------------------------- |
| `nil`, `boolean`, `string`, `number`       | yes        | UTF-8 strings only. NaN and Infinity are refused.                               |
| `table` (sequence 1..n)                    | yes        | Serialized as JSON array.                                                       |
| `table` (string keys)                      | yes        | Serialized as JSON object.                                                      |
| `table` (mixed / sparse / non-string keys) | no         | Refused at serialization time.                                                  |
| `function`, `userdata`, `coroutine`        | no         | Refused. You cannot pass an open socket, file handle, etc. to or from a worker. |
| Cyclic tables                              | no         | Refused immediately on first cycle detection.                                   |

**integer vs float caveat.** JSON has only one number type, so the
integer/float distinction is lost on transfer. A worker returning
`42` (integer) may arrive as `42.0` (float) in the parent, or
vice-versa. If your code checks `math.type(v) == "integer"`, convert
explicitly with `math.tointeger(v)` after the join or recv.

#### What's inside a worker

Every worker gets a fresh `lua_State` with:

* Lua standard library (`io`, `os`, `string`, `table`, `math`,
  `coroutine`, `debug`, `package`).
* The full `luapilot.*` API (`http`, `socket`, `json`, `toml`,
  `sys`, filesystem helpers, hashes, etc.).
* Bundled modules via `require()` (`inspect`, `argparse`, `logging`).
* User modules via `require()` work the same as in the parent
  (resolved from the script directory, or from the embedded ZIP in
  packaged mode).

Two globals are set differently from the parent:

* `worker` is a table containing `args`, `send`, and `recv`.
  `worker.args` holds the table passed to `spawn` (or `nil`).
* `arg` is `nil` (the worker does not inherit the parent's `arg`
  vector). Use `worker.args` instead.

#### Lifecycle and bounded queues

Both queues are **bounded**. The defaults are 64 entries each
(`inbox_capacity` and `outbox_capacity` at spawn time). When a
queue is full, the corresponding `send` blocks (or returns
`"full"` / `"timeout"` according to the timeout argument). This is
intentional **backpressure** — an unbounded queue would silently
grow until memory exhaustion if the consumer is slower than the
producer.

When the worker terminates (return or error), its `worker.send`
outbox is **drained first**: pending messages are still visible
via `w:recv()` until the queue is empty, then `w:recv()` returns
`(false, "closed")`. Symmetrically, `w:send` after worker death
returns `(false, "closed")` immediately.

If the parent never calls `w:join()` or `w:poll()`, the userdata's
`__gc` joins the thread automatically (closing both queues to
unblock any pending operations). This is a safety net, not a
substitute for explicit lifecycle management.

#### Not in v1 (additive later under SemVer)

* **`spawn(function, args)`** via `string.dump`. The current
  string-based form already covers all use cases; function-form
  would be more ergonomic but has subtleties (upvalues are not
  captured, only top-level locals).
* **`:kill()`** — force-terminating a worker (see "non-interruptive"
  above).
* **Pool of persistent workers** — implementable in pure Lua on top
  of `spawn`. A bundled `workers.pool(N)` may come later.
* **`workers.channel()`** — independent channels for worker-to-worker
  communication. The current model is "parent-routed only".
* **Binary serialization** — JSON is fine for typical payloads.
  A faster binary format may be added if large-volume transfers
  become a real use case.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for any suggestions or improvements.

## License

This project is licensed under the MIT License. See the [LICENSE](https://opensource.org/licenses/MIT) file for details.
