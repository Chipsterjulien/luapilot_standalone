#include "fileIterator.hpp"
#include <iostream>

namespace fs = std::filesystem;

/**
 * @brief Constructs a FileIterator for the given path.
 * @param path The directory path to iterate over.
 * @param recursive If true, iterate recursively through subdirectories.
 */
FileIterator::FileIterator(const std::string& path, bool recursive)
    : closed(false) {
    loadFiles(path, recursive);
    current = files.begin();
}

/**
 * @brief Loads the files from the given path.
 * @param path The directory path to load files from.
 * @param recursive If true, load files recursively from subdirectories.
 */
void FileIterator::loadFiles(const std::string& path, bool recursive) {
    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_regular_file(entry)) {
            files.push_back(entry.path().string());
        }
        if (recursive && fs::is_directory(entry)) {
            loadFiles(entry.path().string(), recursive);
        }
    }
}

/**
 * @brief Returns the next file in the directory.
 * @return The next file as a string.
 */
const std::string& FileIterator::next() {
    return *(current++);
}

/**
 * @brief Checks if there are more files to iterate over.
 * @return True if there are more files, false otherwise.
 */
bool FileIterator::hasNext() const {
    return current != files.end();
}

/**
 * @brief Checks if the iterator is closed.
 * @return True if the iterator is closed, false otherwise.
 */
bool FileIterator::isClosed() const {
    return closed;
}

/**
 * @brief Sets the closed state of the iterator.
 * @param value The new closed state.
 */
void FileIterator::setClosed(bool value) {
    closed = value;
}

/**
 * @brief Gets the next file from the FileIterator.
 * @param L The Lua state.
 * @return The number of return values.
 */
int lua_nextFile(lua_State* L) {
    FileIterator* iterator = (FileIterator*)luaL_checkudata(L, 1, "FileIterator");

    if (iterator->isClosed()) {
        lua_pushnil(L);
        return 1;
    }

    if (iterator->hasNext()) {
        lua_pushstring(L, iterator->next().c_str());
    } else {
        iterator->setClosed(true);
        lua_pushnil(L);
    }

    return 1;
}

/**
 * @brief Garbage collects the FileIterator.
 * @param L The Lua state.
 * @return The number of return values.
 */
int lua_gcFileIterator(lua_State* L) {
    FileIterator* iterator = (FileIterator*)luaL_checkudata(L, 1, "FileIterator");
    iterator->~FileIterator();
    return 0;
}

/**
 * @brief Creates a new FileIterator and pushes it onto the Lua stack.
 * @param L The Lua state.
 * @return The number of return values.
 */
int lua_createFileIterator(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool recursive = lua_gettop(L) >= 2 && lua_toboolean(L, 2);

    void* userdata = lua_newuserdata(L, sizeof(FileIterator));
    new (userdata) FileIterator(path, recursive);

    luaL_getmetatable(L, "FileIterator");
    lua_setmetatable(L, -2);

    return 1;
}

/**
 * @brief Creates the metatable for FileIterator.
 * @param L The Lua state.
 * @return The number of return values.
 */
int file_iterator_create_meta(lua_State *L) {
    luaL_newmetatable(L, "FileIterator");

    lua_newtable(L);
    lua_pushcfunction(L, lua_nextFile);
    lua_setfield(L, -2, "next");
    lua_pushcfunction(L, lua_gcFileIterator);
    lua_setfield(L, -2, "close");

    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lua_gcFileIterator);
    lua_setfield(L, -2, "__gc");

    return 1;
}

/**
 * @brief Opens the file_iterator module in Lua.
 * @param L The Lua state.
 * @return The number of return values.
 */
extern "C" int luaopen_file_iterator(lua_State* L) {
    file_iterator_create_meta(L);

    luaL_Reg file_iterator_functions[] = {
        {"createFileIterator", lua_createFileIterator},
        {NULL, NULL}
    };

    luaL_newlib(L, file_iterator_functions);

    return 1;
}
