# luapilot

LuaPilot is a Lua library offering advanced functionalities for file, table, and string manipulation, as well as performance analysis tools. LuaPilot is written in C++ for optimal performance and can be easily integrated into your Lua scripts.

## Features

- **Hello there**

- **File Manipulation**
  - `listFiles`: List files in a directory.
  - `listFilesRecursive`: Recursively list files in a directory.
  - `fileExists`: Check if a file exists.

- **Table Manipulation**
  - `mergeTables`: Merge two Lua tables into one.
  - `mergeMultipleTables`: Merge an arbitrary number of Lua tables into one.
  - `shallowCopy`: Make a shallow copy of a Lua table.
  - `deepCopy`: Make a deep copy of a Lua table.

- **String Manipulation**
  - `splitString`: Split a string based on a delimiter.

## Installation


1. **Clone the repository:**
   ```sh
   git clone https://github.com/your-username/LuaPilot.git
   cd LuaPilot
2. **Build the project locally:**
```
  ./build_local.sh
```

## Usage
The main file is main.lua. This file is the begining of lua script

### To create an executable from a Lua script using the --create-exe option:
```
  # for example :
  echo "print(helloThere())" > main.lua
  luapilot --create-exe . luapilot_with_script
```

### Hello there
```
print(luapilot.helloThere())
```

### File Manipulation
```
-- List files in a directory
local files = luapilot.listFiles("/path/to/directory")
for _, file in ipairs(files) do
    print(file)
end

-- Check if a file exists
local exists = luapilot.fileExists("/path/to/file.txt")
print("File exists:", exists)
```

### Table Manipulation
```
-- Merge two tables
local table1 = {a = 1, b = 2}
local table2 = {b = 3, c = 4}
local mergedTable = luapilot.mergeTables(table1, table2)

-- Merge multiple tables
local mergedMultipleTables = luapilot.mergeTables(table1, table2, {d = 5}, {e = 6})

-- Make a shallow copy of a table
local shallowCopy = luapilot.shallowCopy(table1)

-- Make a deep copy of a table
local deepCopy = luapilot.deepCopy(table1)
```

### String Manipulation
```
-- Split a string
local parts = luapilot.splitString("hello,world", ",")
for _, part in ipairs(parts) do
    print(part)
end
```

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for any suggestions or improvements.

## License

This project is licensed under the MIT License. See the [LICENSE](https://opensource.org/licenses/MIT) file for details.
