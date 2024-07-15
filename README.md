# luaPilot

LuaPilot is a binary that provides advanced functionalities such as splitting strings, merging tables, listing files with or without an iterator, and more. It is written in C++17 for optimal performance and allows embedding a ZIP file into the executable if you want. Upon execution, the ZIP file will be decompressed and loaded into memory.

It is currently compiled to work with Lua 5.4.7, but you can change the version by recompiling the code.

See the **[examples](https://github.com/Chipsterjulien/luapilot_standalone/tree/main/examples)** if you want to learn more …

## Download the latest version

To download the latest version of LuaPilot, visit the [releases page on GitHub](https://github.com/Chipsterjulien/luapilot_standalone/releases) and download the most recent release.

## Compilation

If you want to compile the project, do this :

1. **Clone the repository:**

```sh
git clone https://github.com/Chipsterjulien/LuaPilot.git
cd LuaPilot
```

1. **Build the project locally:**

```sh
  ./build_local.sh
```

## Usage

The main file is main.lua. This file is the begining of lua script. You can load other lua file with require

```sh
  ./luapilot .
```

If zip is embedded :

```sh
  ./luapilot_with_zip_embedded
```

### To create an executable from a Lua script using the --create-exe option

If luapilot is installed globally :

```sh
  # for example :
  echo "print(helloThere())" > main.lua
  luapilot --create-exe . luapilot_with_lua_script_embedded
  ./luapilot_with_zip_embedded
```

If luapilot is not installed globally :

```sh
  # for example :
  echo "print(helloThere())" > main.lua
  ./luapilot --create-exe . luapilot_with_lua_script_embedded
  ./luapilot_with_zip_embedded
```

In this example, LuaPilot creates a ZIP file from the current directory since `.` is the first argument.
  
If you want use your own ZIP:

```sh
  cat luapilot my_file.zip > my_new_exec_program
  chmod +x my_new_exec_program
  ./my_new_exec
```

## Some examples

### Hello there

```lua
print(luapilot.helloThere())
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
local fileFound, err = luapilot.fileExists("my/folder/file.txt")
print(err or "File exists: " .. tostring(fileFound))
```

### Table manipulations

```lua
local inspect = require('inspect')
-- Merge two tables
local table1 = {a = 1, b = 2}
local table2 = {b = 3, c = 4}
local table3 = {"g", "h", "i"}
local t3 = {i = 8, j = { k = 9, l = 10} }
local t4 = { i = 8, j = { k = 9, l = 10, m = { n = 11, o = 12, } } }
local mergedTable = luapilot.mergeTables(table1, table2)
print(inspect(mergedTable))

-- Merge multiple tables
local mergedMultipleTables = luapilot.mergeTables(table1, table2, {d = 5}, {e = 6}, t4, t3)
print(inspect(mergedMultipleTables))

-- Make a deep copy of a table
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

See the [examples](https://github.com/Chipsterjulien/luapilot_standalone/tree/main/examples) if you want to learn more …

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for any suggestions or improvements.

## License

This project is licensed under the MIT License. See the [LICENSE](https://opensource.org/licenses/MIT) file for details.
