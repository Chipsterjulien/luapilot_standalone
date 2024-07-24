#include "fileSize.hpp"
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

/**
 * @brief Check if a file exists and is a regular file, and get its size.
 *
 * @param path The file path to check.
 * @param size Reference to store the size of the file if it exists and is a regular file.
 * @return bool True if the file exists and is a regular file, false otherwise.
 */
bool getFileSize(std::string_view path, uintmax_t& size) {
    std::error_code ec;
    fs::path filePath(path);

    if (fs::exists(filePath, ec) && fs::is_regular_file(filePath, ec)) {
        size = fs::file_size(filePath, ec);
        return !ec;
    }
    return false;
}

/**
 * @brief Lua binding for getting the size of a file.
 *
 * This function is a Lua binding that exposes the getFileSize function to Lua scripts.
 * It returns the file size if the file exists and is a regular file, or an error message if it fails.
 *
 * @param L The Lua state.
 * @return int Number of return values (2: file size and error message or nil).
 */
int lua_fileSize(lua_State* L) {
    if (lua_gettop(L) != 1 || !lua_isstring(L, 1)) {
        return luaL_error(L, "Expected one string argument");
    }

    std::string_view path = luaL_checkstring(L, 1);
    uintmax_t size;
    if (getFileSize(path, size)) {
        lua_pushinteger(L, static_cast<lua_Integer>(size));
        lua_pushnil(L);
    } else {
        std::error_code ec;
        fs::path filePath(path);
        bool fileExists = fs::exists(filePath, ec); // Check existence and get the error code
        std::string error_message = ec ? "Error: " + ec.message() : (fileExists ? "File is not a regular file." : "File does not exist.");
        lua_pushnil(L);
        lua_pushstring(L, error_message.c_str());
    }

    return 2;
}
