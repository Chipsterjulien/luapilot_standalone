> **English** | [Français](../../fr/modules/argparse.md)

# `argparse` — command-line argument parser (Lua module)

A declarative argument parser for command-line scripts. Bundled as
a pure-Lua module, loaded via `require("argparse")`. Inspired by
Python's `argparse` but adapted to Lua idioms.

## Why

A LuaPilot script that takes command-line arguments shouldn't have
to reinvent argument parsing (positional vs flag, defaults, help
text, type coercion, repeated flags) every time. `argparse` makes
the common CLI patterns one-liners.

## API

```lua
local argparse = require("argparse")
local p = argparse("script-name", "One-line description")
```

| Method | Effect |
| --- | --- |
| `p:argument(name)` | declare a positional argument |
| `p:option(name)` | declare a flag (`-x` / `--xxx`) that takes a value |
| `p:flag(name)` | declare a boolean flag (no value) |
| `p:parse()` | parse `arg` table, return parsed table or call `os.exit(1)` on error |

Each declaration returns a builder you can chain :

- `:description("text")` — text shown in `--help`.
- `:default(value)` — default if not given.
- `:convert(fn)` — function to coerce the raw string (`tonumber`, etc.).
- `:count("*"|"+"|"?")` — repeated flags ; defaults to once.
- `:choices({...})` — restrict to a finite set.

## Quick example

```lua
local argparse = require("argparse")

local p = argparse("myapp", "Process some files.")
p:argument("input"):description("Input file")
p:option("-o --output"):description("Output file"):default("out.txt")
p:option("-n --count"):description("How many"):convert(tonumber):default(1)
p:flag("-v --verbose"):description("Verbose output")

local args = p:parse()
print(args.input, args.output, args.count, args.verbose)
```

Running it :

```
$ ./myapp.lua data.csv --count 5 -v
data.csv    out.txt    5    true

$ ./myapp.lua --help
Usage: myapp [-o <output>] [-n <count>] [-v] <input>

Process some files.

Arguments:
   input              Input file

Options:
   -o, --output       Output file (default: out.txt)
   -n, --count        How many (default: 1)
   -v, --verbose      Verbose output
   -h, --help         Show this help message and exit
```

## Error contract

- **Parser construction** (`argparse(...)`) and declarations
  (`p:argument(...)` etc.) : raise via `error()` on misuse.
- **`p:parse()`** : on a malformed command line, prints the error
  + usage to stderr and calls `os.exit(1)`. This matches user
  expectations for CLI tools (no need to handle errors in the
  script).

## Design decisions

- **Bundled as Lua, not C++**. Argument parsing is pure string
  logic, no syscalls. Pure Lua keeps it monkey-patchable for
  unusual needs (custom help format, etc.).
- **Hard exit on parse error**. CLI scripts typically want to die
  with a usage message, not catch errors. If you need to catch
  them, wrap the call in `pcall`.
- **`-h` / `--help` is automatic**. Always present, prints usage
  and exits 0.

## Not in v1

- Sub-commands (`git commit` / `git push` style). May be added
  if a real use case arises.
- Argument groups (visual grouping in help). Cosmetic only.

The reference implementation is bundled from the upstream
[argparse.lua](https://github.com/mpeterv/argparse) project. See
its docs for advanced patterns (mutually exclusive groups, custom
actions, etc.).
