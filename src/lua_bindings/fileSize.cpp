#include "lua_bindings/fileSize.hpp"
#include <filesystem>
#include <cstdint>
#include <stdexcept>

namespace fs = std::filesystem;

bool fileSize(const std::string &path, std::uint64_t &size)
{
    if (fs::exists(path) && fs::is_regular_file(path))
    {
        size = static_cast<std::uint64_t>(fs::file_size(path));
        return true;
    }
    return false;
}

int lua_fileSize(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        return luaL_error(L, "Expected a string as the first argument");
    }

    const char *path = luaL_checkstring(L, 1);
    std::uint64_t size;
    bool success = fileSize(path, size);

    if (!success)
    {
        return luaL_error(L, "File does not exist or is not a regular file");
    }

    lua_pushinteger(L, static_cast<lua_Integer>(size));
    return 1;
}
