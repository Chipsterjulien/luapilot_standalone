#include "fileUtils.hpp"
#include <filesystem>
#include <system_error>
#include <tuple>

namespace fs = std::filesystem;

/**
 * Get the basename (filename with extension) from a full path.
 * @param fullPath The full path of the file.
 * @return A tuple with the basename and an error code if any.
 */
std::tuple<std::string, std::error_code> getBasename(const std::string& fullPath) {
    std::error_code ec;
    auto path = fs::path(fullPath).filename().string();
    return std::make_tuple(path, ec);
}

/**
 * Get the extension of the file from a full path.
 * @param fullPath The full path of the file.
 * @return A tuple with the extension and an error code if any.
 */
std::tuple<std::string, std::error_code> getExtension(const std::string& fullPath) {
    std::error_code ec;
    auto ext = fs::path(fullPath).extension().string();
    return std::make_tuple(ext, ec);
}

/**
 * Get the filename without extension from a full path.
 * @param fullPath The full path of the file.
 * @return A tuple with the filename without extension and an error code if any.
 */
std::tuple<std::string, std::error_code> getFilename(const std::string& fullPath) {
    std::error_code ec;
    auto stem = fs::path(fullPath).stem().string();
    return std::make_tuple(stem, ec);
}

/**
 * Get the parent path from a full path.
 * @param fullPath The full path of the file.
 * @return A tuple with the parent path and an error code if any.
 */
std::tuple<std::string, std::error_code> getPath(const std::string& fullPath) {
    std::error_code ec;
    auto parent = fs::path(fullPath).parent_path().string();
    return std::make_tuple(parent, ec);
}

/**
 * Lua binding for getting the basename.
 * @param L The Lua state.
 * @return Number of return values (2: basename, error).
 * Lua usage: basename, err = lua_getBasename(path)
 */
int lua_getBasename(lua_State* L) {
    // Check that one parameter is passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check that the first argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as the first argument");
    }

    // Retrieve the file path from the Lua argument
    const char* path = luaL_checkstring(L, 1);
    // Get the basename and error code from the full path
    auto [basename, error] = getBasename(path);
    // Push the basename onto the Lua stack as a string
    lua_pushstring(L, basename.c_str());
    // Push the error message or nil onto the Lua stack
    if (error) {
        lua_pushstring(L, error.message().c_str());
    } else {
        lua_pushnil(L); // No error, push nil
    }
    return 2; // Return 2 to indicate two results are returned
}

/**
 * Lua binding for getting the extension.
 * @param L The Lua state.
 * @return Number of return values (2: extension, error).
 * Lua usage: extension, err = lua_getExtension(path)
 */
int lua_getExtension(lua_State* L) {
    // Check that one parameter is passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check that the first argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as the first argument");
    }

    // Retrieve the file path from the Lua argument
    const char* path = luaL_checkstring(L, 1);
    // Get the extension and error code from the full path
    auto [extension, error] = getExtension(path);
    // Push the extension onto the Lua stack as a string
    lua_pushstring(L, extension.c_str());
    // Push the error message or nil onto the Lua stack
    if (error) {
        lua_pushstring(L, error.message().c_str());
    } else {
        lua_pushnil(L); // No error, push nil
    }
    return 2; // Return 2 to indicate two results are returned
}

/**
 * Lua binding for getting the filename without extension.
 * @param L The Lua state.
 * @return Number of return values (2: filename without extension, error).
 * Lua usage: filename, err = lua_getFilename(path)
 */
int lua_getFilename(lua_State* L) {
    // Check that one parameter is passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check that the first argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as the first argument");
    }

    // Retrieve the file path from the Lua argument
    const char* path = luaL_checkstring(L, 1);
    // Get the filename without extension and error code from the full path
    auto [filename, error] = getFilename(path);
    // Push the filename onto the Lua stack as a string
    lua_pushstring(L, filename.c_str());
    // Push the error message or nil onto the Lua stack
    if (error) {
        lua_pushstring(L, error.message().c_str());
    } else {
        lua_pushnil(L); // No error, push nil
    }
    return 2; // Return 2 to indicate two results are returned
}

/**
 * Lua binding for getting the parent path.
 * @param L The Lua state.
 * @return Number of return values (2: parent path, error).
 * Lua usage: parentPath, err = lua_getPath(path)
 */
int lua_getPath(lua_State* L) {
    // Check that one parameter is passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check that the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    // Retrieve the file path from the Lua argument
    const char* path = luaL_checkstring(L, 1);
    // Get the parent path and error code from the full path
    auto [parentPath, error] = getPath(path);
    // Push the parent path onto the Lua stack as a string
    lua_pushstring(L, parentPath.c_str());
    // Push the error message or nil onto the Lua stack
    if (error) {
        lua_pushstring(L, error.message().c_str());
    } else {
        lua_pushnil(L); // No error, push nil
    }
    return 2; // Return 2 to indicate two results are returned
}
