inspect = require("inspect")

print("---")
print("listFiles(path, [optional] recursive)")
print("show files only in current directory (no recursive mode)")
local files
files, err = luapilot.listFiles(".")
if err then
    print(err)
else
    for _, file in ipairs(files) do
        print(file)
    end
end

print("---")
print("listFiles(path, [optional] recursive)")
print("show files in current directory in recursive mode")
files, err = luapilot.listFiles(".", true)
if err then
    print(err)
else
    for _, file in ipairs(files) do
        print(file)
    end
end

print("---")
print("fileIterator(path, [optional] recursive)")
print("show files in current directory in recursive mode")
local iterator = luapilot.createFileIterator(".", true)

while true do
    local file = iterator:next()
    if not file then
        break
    end
    print(file)
end

print("---")
print("print hello there")
luapilot.helloThere()

print("---")
print("mkdir(path), [optional] ignore_if_exists")
err = luapilot.mkdir("my/folder", true)
if err then
    print(err)
else
    print("Les répertoires 'my/folder' ont été créés avec succès")
end

print("---")
print("touch(path)")
err = luapilot.touch("my/folder/file.txt")
if err then
    print(err)
else
    print("Fichier 'my/folder/file.txt' créé avec succès")
end

print("---")
print("rmdir(path)")
luapilot.mkdir("zz")
err = luapilot.rmdir("zz")
if err then
    print(err)
else
    print("Le répertoire 'zz' a été supprimé avec succès")
end

print("---")
print("chdir(path)")
err = luapilot.chdir("my")
if err then
    print(err)
else
    print("Changement de répertoire pour 'my'")
end

print("---")
print("currentDir()")
local directory
directory, err = luapilot.currentDir()
if err then
    print(err)
else
    print(directory)
end

luapilot.chdir("..")

print("---")
print("fileExists(path)")
local fileFound, err = luapilot.fileExists("my/folder/file.txt")
if fileFound then
    print("Le fichier 'my/folder/file.txt' existe")
    local f = io.open("my/folder/file.txt", "a")
    if f then
        f:write("coin")
        f:close()
    end
else
    print("Le fichier 'my/folder/file.txt' n'existe pas")
end

print("---")
print("fileSize(path)")
local fileSize
fileSize, err = luapilot.fileSize("my/folder/file.txt")
if err then
    print(err)
else
    print("La taille du fichier 'my/folder/file.txt est de " .. fileSize .. "ko")
end

print("---")
print("md5(path)")
local md5sum
md5sum, err = luapilot.md5sum("my/folder/file.txt")
if err then
    print(err)
else
    print("MD5 sum: " .. md5sum)
end

print("---")
print("sha1sum(path)")
local sha1sum
sha1sum, err = luapilot.sha1sum("my/folder/file.txt")
if err then
    print(err)
else
    print("SHA1 sum: " .. sha1sum)
end

print("---")
print("sha3_256sum(path)")
local sha3_256sum
sha3_256sum, err = luapilot.sha3_256sum("my/folder/file.txt")
if err then
    print(err)
else
    print("SHA3_256 sum: " .. sha3_256sum)
end

print("---")
print("sha3_512sum(path)")
local sha3_512sum
sha3_512sum, err = luapilot.sha3_512sum("my/folder/file.txt")
if err then
    print(err)
else
    print("SHA3_512 sum: " .. sha3_512sum)
end

print("---")
print("sha256sum(path)")
local sha256sum
sha256sum, err = luapilot.sha256sum("my/folder/file.txt")
if err then
    print(err)
else
    print("SHA256 sum: " .. sha256sum)
end

print("---")
print("sha512sum(path)")
local sha512sum
sha512sum, err = luapilot.sha512sum("my/folder/file.txt")
if err then
    print(err)
else
    print("SHA512 sum: " .. sha512sum)
end

print("---")
print("link()")
err = luapilot.link("my/folder/file.txt", "my/target")
if err then
    print(err)
else
    print("Symbolic link created successfully")
end

print("---")
print("copyTree(\"/path/to/sourcedir\", \"/path/to/destdir\")")
err = luapilot.copyTree("my", "my2")
if err then
    print(err)
else
    print("CopyTree succeeded")
end

print("---")
luapilot.mkdir("my3")
print("moveTree(source, destination)")
err = luapilot.moveTree("my2", "my3")
if err then
    print(err)
else
    print("MoveTree succeeded")
end

print("---")
print("getMemoryUsage()")
local memUsage = luapilot.getMemoryUsage()
print("Memory usage: " .. memUsage .. "kB")

print("---")
print("setmode(path, mode)")
local path = "my/folder/file.txt"
local mode = 0755 -- Permission in octal format
err = luapilot.setmode(path, mode)
if err then
    print(err)
else
    print("Permissions changées avec succès")
end

print("---")
print("sleep(sleepTime)")
local sleepTime = 2
luapilot.sleep(sleepTime)

print("---")
print("symlinkattr(path, new_owner, new_group)")
--path = "my/folder/file.txt"
local new_owner = 1000
local new_group = 1000
err = luapilot.symlinkattr(path, new_owner, new_group)
if err then
    print(err)
else
    print("Symbolic link attributes changed successfully")
end

print("---")
print("rmdirAll(path)")
err = luapilot.rmdirAll("my")
if err then
    print(err)
else
    print("Le répertoire 'my' et tout ce qu'il contient a été supprimé")
end

print("---")
print("split(myString)")
local helloSplitted = luapilot.split("Hello there !", " ")
print(inspect(helloSplitted))

print("---")

local fullPath = "/home/test/hello.txt"
print("Print string path of " .. fullPath)
local output
output, err = luapilot.getPath(fullPath)
print(output or err)
print("Print basename path of " .. fullPath)
output, err = luapilot.getBasename(fullPath)
print(output or err)
print("Print string extension of " .. fullPath)
output, err = luapilot.getExtension(fullPath)
print(output or err)

print("---")
-- Tester la fonction mergeTable
local table1 = {"a", "b", "c"}
local table2 = {"d", "e", "f"}
local table3 = {"g", "h", "i"}
local t3 = {i = 8, j = { k = 9, l = 10} }
local t4 = { i = 8, j = { k = 9, l = 10, m = { n = 11, o = 12, } } }

print("mergeTales(table1, table2)")
local mergedTables
mergedTables = luapilot.mergeTables(table1, table2, table3, t3, t4)
print(inspect(mergedTables))

print("---")
print("deepCopyTable(t4)")
local test = luapilot.deepCopyTable(t4)
print(inspect(test))

print("---")
print("find(path, {options}) -- Path do not exists")
local options = {
    mindepth = 1,
    maxdepth = 5,
    type = "f",
    name = ".*%.cpp",
    iname = "",
    path = "",
}

local results
results, err = luapilot.find("/path/to/search", options)
if err then
    print(err)
else
    for _, path_ in ipairs(results) do
        print(path_)
    end
end

print("---")
print("find(path, {options}) -- Privilege denied")
results, err = luapilot.find("/root", options)
if err then
    print(err)
else
    for _, path_ in ipairs(results) do
        print(path_)
    end
end

print("---")
print("find(path, {options})")
results, err = luapilot.find("..", options)
if err then
    print(err)
else
    for _, path_ in ipairs(results) do
        print(path_)
    end
end

print("---")
print("rmdir(path)")
local path1 = "test"
err = luapilot.rmdir(path1)
if err then
    print(err)
else
    print("Directory " .. path1 .. " removed successfully.")
end

print("---")
print("rmdirAll(path)")
err = luapilot.rmdirAll(path1)
if err then
    print(err)
else
    print("Directory " .. path1 .. " removed successfully.")
end

luapilot.rmdirAll("my2")

print("---")
print("joinPath(path1, path2, …) or joinPath({path1, path2, …})")
local newPath = luapilot.joinPath({"test", "coin.txt"})
print("newPath: " .. newPath)

newPath = luapilot.joinPath({"test/", "coin.txt"})
print("newPath: " .. newPath)

newPath = luapilot.joinPath({"test", "/coin.txt"})
print("newPath: " .. newPath)

newPath = luapilot.joinPath({"test/", "/coin.txt"})
print("newPath: " .. newPath)

newPath = luapilot.joinPath("test", "coin.txt")
print("newPath: " .. newPath)

newPath = luapilot.joinPath("test/", "coin.txt")
print("newPath: " .. newPath)

newPath = luapilot.joinPath("test", "/coin.txt")
print("newPath: " .. newPath)

newPath = luapilot.joinPath("test/", "/coin.txt")
print("newPath: " .. newPath)

newPath = luapilot.joinPath("test", "path", "coin.txt")
print("newPath: " .. newPath)

newPath = luapilot.joinPath("test", "/path", "coin.txt")
print("newPath: " .. newPath)

newPath = luapilot.joinPath("/test", "/path", "coin.txt")
print("newPath: " .. newPath)

newPath = luapilot.joinPath({"test", "/path", "coin.txt"})
print("newPath: " .. newPath)

newPath = luapilot.joinPath({"/test", "/path", "coin.txt"})
print("newPath: " .. newPath)

print("---")
print("isdir(path)")
local isDir = luapilot.isdir("..")
print("'..' is dir: " .. tostring(isDir))

print("---")
print("isfile(path)")
local isFile = luapilot.isfile("main.lua")
print("'main.lua' is file: " .. tostring(isFile))

print("---")
print("rename(oldName, newName)")
luapilot.touch("essaie")
err = luapilot.rename("essaie", "essaie2")
if err then
    print(err)
else
    print("Rename succeeded")
end

print("---")
print("copy(\"/path/to/sourceFile\", \"/path/to/destinationFile\")")
err = luapilot.copy("essaie2", "essaie1")
if err then
    print(err)
else
    print("Copy succeeded")
end

print("---")
print("remove(filePath)")
err = luapilot.remove("essaie2")
if err then
    print(err)
else
    print("Remove is succeeded")
end
luapilot.remove("essaie1")
luapilot.rmdirAll("my3")
