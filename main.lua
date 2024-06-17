print("Liste des fichiers dans le répertoire courant:")
files = luapilot.listFiles(".")
for i, file in ipairs(files) do
    print(file)
end

luapilot.helloThere()
