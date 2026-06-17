> **English** | [Français](../fr/getting-started.md)

# Getting started

## Download

Prebuilt binaries for x86_64, aarch64 (RPi4), and armv6l (RPi0) are
available on the
[releases page](https://github.com/Chipsterjulien/luapilot_standalone/releases).

## Build from source

LuaPilot vendors all its dependencies (Lua, OpenSSL, SQLite, miniz,
nlohmann/json, cpp-httplib, tomlplusplus), so the only prerequisites
on your system are :

- a C++23 compiler (recent GCC or Clang)
- CMake (≥ 3.20)
- `wget` and `unzip`

```sh
git clone https://github.com/Chipsterjulien/luapilot_standalone.git
cd luapilot_standalone
./build_local.sh        # downloads deps and compiles
                        # (~5 min on first run, faster afterwards)
./run_tests.sh          # offline harness — should print 827 PASS / 0 FAIL
```

The build script downloads each dependency from its upstream source,
verifies the SHA256, then compiles statically. If the upstream is
temporarily unreachable (`lua.org` notably has had outages), the
script falls back to the Internet Archive's Wayback Machine — the
SHA256 check still applies.

The resulting binary is at `test/luapilot`.

## Three ways to run a Lua script

LuaPilot supports three launch modes :

### 1. Folder mode

Point the binary at a directory containing `main.lua` :

```sh
echo 'print("hello from LuaPilot")' > main.lua
./test/luapilot .
```

`require("X")` in `main.lua` looks for `X.lua` in the same directory.

### 2. Embedded mode (single-file executable)

Package a script and its modules into a self-contained binary :

```sh
./test/luapilot --create-exe . myapp
./myapp        # runs main.lua from the embedded ZIP
```

Internally, LuaPilot appends a ZIP to its own binary, and reads
`main.lua` (plus any `require`d module) from that ZIP at runtime.
The resulting `myapp` is fully self-contained — no LuaPilot install
needed on the target machine, just glibc.

### 3. Embedded via PATH (folder + auto-detect)

If LuaPilot is on `$PATH` and you `chmod +x main.lua` after adding
a shebang line :

```lua
#!/usr/bin/env luapilot
print("hello")
```

```sh
chmod +x main.lua
./main.lua
```

LuaPilot uses the script's own directory as the folder, so
`require("helpers")` finds `helpers.lua` next to `main.lua`.

## Command-line invocation

In addition to the three launch modes above, LuaPilot accepts a
few standard flags :

| Flag | Effect |
| --- | --- |
| `-h`, `--help` | Print the help text and exit `0`. |
| `-V`, `--version` | Print `luapilot <version>` and exit `0`. |
| `-c <dir> <out>`, `--create-exe <dir> <out>` | Create a self-contained executable named `<out>` by embedding `<dir>` (which must contain `main.lua`). |

Any other argument starting with `-` is treated as an **unknown
option** : LuaPilot prints `Unknown option: ...` + a hint to use
`--help`, and exits `1` instead of trying to interpret it as a
directory name. Folders whose name legitimately starts with `-`
can still be passed via `./-dirname` (POSIX convention).

```sh
luapilot --version    # luapilot 1.7.1
luapilot --help       # full usage
luapilot --bogus      # Unknown option: --bogus
                      # Try 'luapilot --help' for more information.
```

The same version is also exposed to scripts as `luapilot.VERSION`
(plus `luapilot.VERSION_MAJOR` / `VERSION_MINOR` / `VERSION_PATCH`
as integers) — see [`sys`](modules/sys.md).

## First script

Once the binary is built, try this :

```lua
-- main.lua
print("LuaPilot says hi")
print("PID:", luapilot.pid())
print("Host:", luapilot.hostname())

local r, err = luapilot.http.get("https://example.com/")
if r then
    print("Status:", r.status)
else
    print("HTTP failed:", err)
end
```

Run it with `./test/luapilot .` (or whichever mode you prefer).

## Where to look next

- Each module has its own page under [`modules/`](modules/).
- The [`user`](modules/user.md) module is a small, complete example
  of the documentation pattern used throughout — read it as the
  canonical reference.
- See [`security.md`](security.md) before exposing LuaPilot scripts
  to anything that might receive untrusted input.
