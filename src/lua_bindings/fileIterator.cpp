#include "fileIterator.hpp"
#include <stdexcept>
#include <memory>

namespace fs = std::filesystem;

FileIterator::FileIterator(const std::string& path, bool recursive) {
    loadFiles(path, recursive);
    current = files.begin();
}

void FileIterator::loadFiles(const fs::path& path, bool recursive) {
    std::error_code ec;

    auto addFiles = [&](const auto& iterator) {
        for (const auto& entry : iterator) {
            if (entry.is_regular_file(ec)) {
                files.emplace_back(entry.path().string());
            }
        }
    };

    if (recursive) {
        addFiles(fs::recursive_directory_iterator(path, ec));
    } else {
        addFiles(fs::directory_iterator(path, ec));
    }

    if (ec) {
        throw std::runtime_error("Error accessing directory: " + ec.message());
    }
}

std::optional<std::string> FileIterator::next() {
    if (current != files.end()) {
        return *(current++);
    }
    return std::nullopt;
}

bool FileIterator::hasNext() const {
    return current != files.end();
}

int lua_nextFile(lua_State* L) {
    auto iterator = *static_cast<std::shared_ptr<FileIterator>*>(luaL_checkudata(L, 1, "FileIterator"));

    if (auto nextFile = iterator->next()) {
        lua_pushstring(L, nextFile->c_str());
    } else {
        lua_pushnil(L);
    }

    return 1;
}

int lua_gcFileIterator(lua_State* L) {
    auto iterator = static_cast<std::shared_ptr<FileIterator>*>(luaL_checkudata(L, 1, "FileIterator"));
    iterator->~shared_ptr();
    return 0;
}

int lua_createFileIterator(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool recursive = lua_gettop(L) >= 2 && lua_toboolean(L, 2);

    void* userdata = lua_newuserdata(L, sizeof(std::shared_ptr<FileIterator>));
    try {
        new (userdata) std::shared_ptr<FileIterator>(std::make_shared<FileIterator>(path, recursive));
    } catch (const std::exception& e) {
        return luaL_error(L, "Failed to create FileIterator: %s", e.what());
    }

    luaL_getmetatable(L, "FileIterator");
    lua_setmetatable(L, -2);

    return 1;
}

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

extern "C" int luaopen_file_iterator(lua_State* L) {
    file_iterator_create_meta(L);

    luaL_Reg file_iterator_functions[] = {
        {"createFileIterator", lua_createFileIterator},
        {NULL, NULL}
    };

    luaL_newlib(L, file_iterator_functions);

    return 1;
}
