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
git clone https://github.com/Chipsterjulien/LuaPilot.git
cd LuaPilot
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

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for any suggestions or improvements.

## License

This project is licensed under the MIT License. See the [LICENSE](https://opensource.org/licenses/MIT) file for details.
