> **English** | [Français](../../fr/modules/argparse.md)

# `argparse` — command-line argument parser (Lua module)

A declarative argument parser for command-line scripts. Bundled as
a pure-Lua module, loaded via `require("argparse")`. Returns
results as data (not via prints or `os.exit`) — the script decides
what to do.

## Why

A LuaPilot script that takes command-line arguments shouldn't have
to reinvent argument parsing (positional vs flag, defaults, help
text, type coercion, choices) every time. `argparse` makes the
common CLI patterns one-liners — and stays out of the script's way
by returning everything as data : help is a result, not a side
effect ; errors are a return value, not an exception.

## API

```lua
local argparse = require("argparse")
local p = argparse(prog, description?)
```

| Builder method | Returns | Use |
| --- | --- | --- |
| `p:flag(spec, opts?)` | `p` (chainable) | Boolean flag (no value). |
| `p:option(spec, opts?)` | `p` (chainable) | Flag that takes a value. |
| `p:argument(name, opts?)` | `p` (chainable) | Positional argument. |
| `p:parse(src?)` | `(res, err)` | Parse arguments (see below). |
| `p:get_usage()` | `string` | Render the help text manually. |

`spec` for `flag` and `option` is a **single string** containing
one or more names separated by spaces : `"-v"`, `"--verbose"`,
or `"-v --verbose"`. Names must start with `-`.

`name` for `argument` is a plain identifier (no leading `-`).

### `opts` table fields

| Field | Type | Effect |
| --- | --- | --- |
| `help` | string | Help text shown by `get_usage()`. |
| `default` | any | Value returned when the flag/option/argument is absent. |
| `dest` | string | Force the destination key in `res`. Default: first long name without dashes, else first short name without dash. |
| `required` | boolean | Mark as required. For positionals, defaults to `true` unless `default` is set. |
| `choices` | array of strings | Restrict the raw value to a finite set. |
| `convert` | function `string -> any` | Coerce the raw string. Returning `nil` is treated as a failure (the convert function may return `(nil, "reason")`). |

### `parse(src?)` return contract

- **Success** : `(res, nil)` — `res` is a table indexed by `dest`,
  containing the parsed value (or default) for each declared
  flag / option / argument.
- **Help requested** : `({ help = true, usage = "<text>" }, nil)`
  when the user passed `-h` or `--help`. Help is a **success**,
  not an error. The script decides what to do (typically print
  `res.usage` and return).
- **Failure** : `(nil, err)` — human-readable string for any user
  input problem (unknown option, missing value, invalid choice,
  conversion error, missing required argument, extra positional).

`parse()` **never raises** on user input — every user error goes
through `(nil, err)`. Only **builder misuse** (empty spec, name
without `-`, duplicate name, etc.) raises via `error()`, because
that's a programmer bug.

### Source of arguments

- `p:parse()` (no argument) reads the global `arg` table, indices
  `1..n` (stopping at the first hole). Indices `<= 0` (binary path,
  folder) are ignored — these are LuaPilot's internal slots, not
  user arguments.
- `p:parse({ "a", "b" })` parses an explicit array. Useful for
  testing or for parsing arguments that don't come from the shell.

### Accepted argument forms

- `--long val`
- `--long=val`
- `-s val`
- `-s=val`
- `--` ends option parsing — every subsequent token is treated
  as a positional, even if it starts with `-`.

Short options collapsed together (`-abc`) and the `-fvalue`
form (short option immediately followed by its value, no
separator) are **not supported** in v1.

## Quick example

```lua
local argparse = require("argparse")

local p = argparse("myapp", "Process some files.")
    :flag("-v --verbose", { help = "Verbose output" })
    :option("-o --output",
            { help = "Output file", default = "out.txt" })
    :option("-n --count",
            { help = "How many", convert = tonumber, default = 1 })
    :argument("input", { help = "Input file" })

local args, err = p:parse()

-- Help is a success, not an error.
if args and args.help then
    print(args.usage)
    return
end

if err then
    io.stderr:write("error: ", err, "\n")
    io.stderr:write(p:get_usage(), "\n")
    os.exit(2)
end

print(args.input, args.output, args.count, args.verbose)
```

Running it :

```
$ ./myapp.lua data.csv --count 5 -v
data.csv    out.txt    5    true

$ ./myapp.lua --help
Usage: myapp [options] <input>

Process some files.

Arguments:
  input  Input file

Options:
  -h, --help        show this help
  -v, --verbose     Verbose output
  -o, --output <value>  Output file
  -n, --count <value>   How many
```

## Error contract

- **Builder misuse** (`p:flag("")`, `p:option("foo")` without `-`,
  duplicate name, `p:argument("-bad")`) → raises via `error()`.
  These are programmer bugs.
- **`p:parse(...)`** never raises on user input. All user input
  problems are reported as `(nil, "message")` :
  - `"unknown option '--xxx'"`
  - `"option '--xxx' requires a value"`
  - `"flag '-v' does not take a value"`
  - `"missing required option '--xxx'"`
  - `"missing required argument 'name'"`
  - `"argument 'name': invalid choice 'xxx'"`
  - `"option '--xxx': <converter message>"`
  - `"unexpected argument 'xxx'"`
- A `convert` function that itself raises is **caught** by
  `parse()` and turned into a `(nil, "<...>: conversion error")`.
  The script never sees an exception coming out of `parse()`.

## Design decisions

- **Help is a success, not a side effect**. Returning
  `{ help = true, usage = "..." }` instead of printing and exiting
  lets the script decide : print to stdout, log to a file, send
  to a GUI, prepend a banner, whatever. The library doesn't
  decide for you, consistent with the rest of LuaPilot where no
  module unilaterally exits the process.
- **`(res, err)` everywhere on user input, `error()` only on
  builder bugs**. Same convention as the rest of LuaPilot (`user`,
  `json`, `sqlite`, etc.).
- **Spec is a single string** (`"-v --verbose"`), not a chain of
  calls. Discoverable, terse, and avoids ambiguity when multiple
  names are declared together.
- **`choices` is checked on the raw string, then `convert`
  applied**. If you pair them, express `choices` as strings — e.g.
  `{ choices = {"1","2","3"}, convert = tonumber }`. Documented
  trap, but conscious : doing the reverse would force `choices`
  values to be of the converted type, which couples two
  independent features.
- **Positional with `default` is implicitly optional**. Saves the
  `required = false` boilerplate in the common case.
- **`parse()` reads `arg[1..n]`** — never `arg[0]` or negative
  indices, where LuaPilot puts its own slots. Just what the user
  actually typed.

## Not in v1

Additive — could be added later without breaking SemVer :

- Sub-commands (`git commit` / `git push` style).
- Combined short options (`-abc` for `-a -b -c`).
- Short options with attached values (`-fvalue`).
- Variadic positionals (`nargs = "+"` or `"*"`).
- Argument groups (visual grouping in help).
