#include "attributes.hpp"
#include <filesystem>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>  // Pour errno

namespace fs = std::filesystem;

/**
 * @brief Utility function to check and retrieve arguments from Lua stack.
 *
 * @param L Lua state.
 * @param expected_args The expected number of arguments.
 * @param index The index of the argument.
 * @param type The expected type of the argument.
 */
void check_arg(lua_State* L, int expected_args, int index, int type) {
    luaL_argcheck(L, lua_gettop(L) == expected_args, index, "Incorrect number of arguments");
    luaL_argcheck(L, lua_type(L, index) == type, index, lua_typename(L, type));
}

/**
 * @brief Utility function to validate and push an attributes table to Lua stack.
 *
 * @param L Lua state.
 * @param perms The file permissions.
 * @param owner The owner UID.
 * @param group The group GID.
 * @return int Returns 1 if an error occurred, 0 otherwise.
 */
int push_attributes(lua_State* L, fs::perms perms, uid_t owner, gid_t group) {
    // Validate UID and GID
    if (owner < 0 || group < 0) {
        return push_error(L, "UID and GID must be non-negative integers");
    }

    // Validate permissions
    if ((static_cast<int>(perms) & ~0777) != 0) {
        return push_error(L, "Invalid permissions value");
    }

    lua_newtable(L);

    lua_pushstring(L, "mode");
    lua_pushinteger(L, static_cast<int>(perms) & 0777); // Mask to get only the permission bits
    lua_settable(L, -3);

    lua_pushstring(L, "owner");
    lua_pushinteger(L, owner);
    lua_settable(L, -3);

    lua_pushstring(L, "group");
    lua_pushinteger(L, group);
    lua_settable(L, -3);

    return 0; // No error
}

/**
 * @brief Changes the owner, group, and optionally the permissions of a file or directory.
 *
 * @param path The directory path to change to.
 * @param owner The new owner (UID).
 * @param group The new group (GID).
 * @param mode Optional new permissions (mode).
 * @return std::optional<std::string> An optional string containing an error message if any, or an empty optional if successful.
 */
std::optional<std::string> set_attributes(const std::filesystem::path& path, uid_t owner, gid_t group, std::optional<fs::perms> mode) {
    // Validate UID and GID
    if (owner < 0 || group < 0) {
        return "UID and GID must be non-negative integers";
    }

    // Change owner and group
    if (chown(path.c_str(), owner, group) != 0) {
        switch (errno) {
            case EACCES:
                return "Permission denied";
            case ELOOP:
                return "Too many symbolic links encountered";
            case ENAMETOOLONG:
                return "File name too long";
            case ENOENT:
                return "No such file or directory";
            case ENOTDIR:
                return "Not a directory";
            case EPERM:
                return "Operation not permitted";
            case EROFS:
                return "Read-only file system";
            default:
                return std::generic_category().message(errno);
        }
    }

    // Change permissions if specified
    if (mode) {
        try {
            fs::permissions(path, *mode);
        } catch (const fs::filesystem_error& e) {
            return e.what();
        }
    }

    return std::nullopt;
}

/**
 * @brief Lua binding for changing the owner, group, and optionally the permissions of a file or directory.
 *
 * @param L The Lua state.
 * @return int Number of return values (1: error message or nil).
 */
int lua_setattr(lua_State* L) {
    // Check and retrieve arguments
    if (lua_gettop(L) < 3 || lua_gettop(L) > 4) {
        return luaL_error(L, "Expected three or four arguments");
    }
    check_arg(L, lua_gettop(L), 1, LUA_TSTRING);
    check_arg(L, lua_gettop(L), 2, LUA_TNUMBER);
    check_arg(L, lua_gettop(L), 3, LUA_TNUMBER);

    const char* path = luaL_checkstring(L, 1);
    uid_t owner = static_cast<uid_t>(luaL_checkinteger(L, 2));
    gid_t group = static_cast<gid_t>(luaL_checkinteger(L, 3));

    std::optional<fs::perms> mode = std::nullopt;
    if (lua_gettop(L) == 4) {
        check_arg(L, lua_gettop(L), 4, LUA_TNUMBER);
        mode = static_cast<fs::perms>(luaL_checkinteger(L, 4));
    }

    auto result = set_attributes(path, owner, group, mode);
    if (result) {
        return push_error(L, *result); // Error, push error message
    } else {
        lua_pushnil(L); // Success, push nil
        return 1;
    }
}

/**
 * @brief Gets the attributes of a file or directory.
 *
 * This function takes one argument from the Lua stack:
 * 1. The path to the file or directory.
 *
 * If it fails, it returns the error message to Lua.
 * If successful, it returns a table with the attributes to Lua.
 *
 * @param L Lua state.
 * @return int Number of return values on the Lua stack.
 */
int lua_getattr(lua_State* L) {
    // Check and retrieve arguments
    check_arg(L, 1, 1, LUA_TSTRING);

    const char* path = luaL_checkstring(L, 1);
    struct stat fileStat;

    // Get file status
    if (stat(path, &fileStat) != 0) {
        switch (errno) {
            case EACCES:
                return push_error(L, "Permission denied");
            case ELOOP:
                return push_error(L, "Too many symbolic links encountered");
            case ENAMETOOLONG:
                return push_error(L, "File name too long");
            case ENOENT:
                return push_error(L, "No such file or directory");
            case ENOTDIR:
                return push_error(L, "Not a directory");
            case EROFS:
                return push_error(L, "Read-only file system");
            default:
                return push_error(L, std::generic_category().message(errno));
        }
    }

    // Get permissions using filesystem library
    std::error_code ec;
    fs::perms perms = fs::status(path, ec).permissions();
    if (ec) {
        return push_error(L, ec.message());
    }

    // Push attributes to Lua stack
    if (push_attributes(L, perms, fileStat.st_uid, fileStat.st_gid) != 0) {
        return 1; // Return error if validation fails
    }

    return 1; // Return the table
}
