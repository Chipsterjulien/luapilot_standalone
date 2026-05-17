#include "fileIterator.hpp"
#include "lua_utils.hpp"
#include <stdexcept>
#include <memory>

namespace fs = std::filesystem;

FileIterator::FileIterator(const std::string &path, bool recursive)
{
    loadFiles(path, recursive);
    current = files.begin();
}

void FileIterator::loadFiles(const fs::path &path, bool recursive)
{
    std::error_code ec;

    auto addFiles = [&](const auto &iterator)
    {
        for (const auto &entry : iterator)
        {
            if (entry.is_regular_file(ec))
            {
                files.emplace_back(entry.path().string());
            }
        }
    };

    if (recursive)
    {
        addFiles(fs::recursive_directory_iterator(path, ec));
    }
    else
    {
        addFiles(fs::directory_iterator(path, ec));
    }

    if (ec)
    {
        throw std::runtime_error("cannot access directory: " + ec.message());
    }
}

std::optional<std::string> FileIterator::next()
{
    if (current != files.end())
    {
        return *(current++);
    }
    return std::nullopt;
}

bool FileIterator::hasNext() const
{
    return current != files.end();
}

static std::shared_ptr<FileIterator> *check_iterator(lua_State *L)
{
    return static_cast<std::shared_ptr<FileIterator> *>(
        luaL_checkudata(L, 1, "FileIterator"));
}

int lua_nextFile(lua_State *L)
{
    auto *iter = check_iterator(L);
    if (!*iter)
    {
        return luaL_error(L, "iterator has been closed");
    }
    if (auto nextFile = (*iter)->next())
    {
        lua_pushstring(L, nextFile->c_str());
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_closeFileIterator(lua_State *L)
{
    auto *iter = check_iterator(L);
    iter->reset();
    return 0;
}

int lua_gcFileIterator(lua_State *L)
{
    auto *iter = check_iterator(L);
    iter->~shared_ptr();
    return 0;
}

int lua_createFileIterator(lua_State *L)
{
    const char *path = luaL_checkstring(L, 1);
    bool recursive = lua_gettop(L) >= 2 && lua_toboolean(L, 2);

    // Construire l'objet d'abord : si ça throw, la pile Lua n'a pas été touchée.
    std::shared_ptr<FileIterator> iter;
    try
    {
        iter = std::make_shared<FileIterator>(path, recursive);
    }
    catch (const std::exception &e)
    {
        return push_fail(L, e.what());
    }

    void *userdata = lua_newuserdata(L, sizeof(std::shared_ptr<FileIterator>));
    new (userdata) std::shared_ptr<FileIterator>(std::move(iter));

    luaL_getmetatable(L, "FileIterator");
    lua_setmetatable(L, -2);

    lua_pushnil(L); // pas d'erreur
    return 2;
}

int file_iterator_create_meta(lua_State *L)
{
    luaL_newmetatable(L, "FileIterator");

    lua_newtable(L);
    lua_pushcfunction(L, lua_nextFile);
    lua_setfield(L, -2, "next");
    lua_pushcfunction(L, lua_closeFileIterator);
    lua_setfield(L, -2, "close");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, lua_gcFileIterator);
    lua_setfield(L, -2, "__gc");
    return 1;
}

extern "C" int luaopen_file_iterator(lua_State *L)
{
    file_iterator_create_meta(L);

    luaL_Reg file_iterator_functions[] = {
        {"createFileIterator", lua_createFileIterator},
        {NULL, NULL}};

    luaL_newlib(L, file_iterator_functions);
    return 1;
}