#include "lua_bindings/find.hpp"
#include <regex>
#include <limits>
#include <iostream>

/**
 * @brief Transforms Lua regex escape sequences to C++ regex escape sequences.
 *
 * This function takes a Lua regex pattern string and converts any Lua-specific
 * escape sequences (e.g., %.) to their C++ regex equivalents (e.g., \.).
 *
 * @param lua_regex The Lua regex pattern string.
 * @return The transformed C++ regex pattern string.
 */
std::string transform_lua_regex(const std::string& lua_regex) {
    std::string cpp_regex;
    cpp_regex.reserve(lua_regex.size());

    for (size_t i = 0; i < lua_regex.size(); ++i) {
        if (lua_regex[i] == '%') {
            cpp_regex.push_back('\\');
            if (i + 1 < lua_regex.size()) {
                cpp_regex.push_back(lua_regex[i + 1]);
                ++i;
            }
        } else {
            cpp_regex.push_back(lua_regex[i]);
        }
    }

    return cpp_regex;
}

/**
 * @brief Recursively searches for files and directories based on the provided options.
 *
 * This function performs a recursive search starting from the given root directory,
 * applying the specified options to filter the results. The search results are stored
 * in the provided vector.
 *
 * @param root The root directory to start the search from.
 * @param options The search options, including filters and depth limits.
 * @param depth The current depth of the search.
 * @param results The vector to store the search results.
 */
void find(const fs::path& root, const FindOptions& options, int depth, std::vector<fs::path>& results) {
    if (!fs::exists(root)) {
        std::cerr << "Error: Path does not exist: " << root << std::endl;
        return;
    }

    if (!fs::is_directory(root)) {
        std::cerr << "Error: Path is not a directory: " << root << std::endl;
        return;
    }

    if (depth > options.maxdepth) return;

    try {
        for (const auto& entry : fs::directory_iterator(root)) {
            if (depth >= options.mindepth) {
                bool matches = true;

                if (!options.type.empty()) {
                    if ((options.type == "f" && !fs::is_regular_file(entry)) ||
                        (options.type == "d" && !fs::is_directory(entry))) {
                        matches = false;
                    }
                }

                if (!options.name.empty()) {
                    std::string name_pattern = transform_lua_regex(options.name);
                    std::regex name_regex(name_pattern);
                    if (!std::regex_match(entry.path().filename().string(), name_regex)) {
                        matches = false;
                    }
                }

                if (!options.iname.empty()) {
                    std::string iname_pattern = transform_lua_regex(options.iname);
                    std::regex iname_regex(iname_pattern, std::regex_constants::icase);
                    if (!std::regex_match(entry.path().filename().string(), iname_regex)) {
                        matches = false;
                    }
                }

                if (!options.path.empty()) {
                    std::string path_pattern = transform_lua_regex(options.path);
                    std::regex path_regex(path_pattern);
                    if (!std::regex_match(entry.path().string(), path_regex)) {
                        matches = false;
                    }
                }

                if (matches) {
                    results.push_back(entry.path());
                }
            }

            if (fs::is_directory(entry)) {
                find(entry.path(), options, depth + 1, results);
            }
        }
    } catch (const fs::filesystem_error& e) {
        throw;
    }
}

/**
 * @brief Lua binding for the find function.
 *
 * This function is a Lua binding for the find function, allowing it to be called from Lua scripts.
 * It retrieves the search options from the Lua stack, performs the search, and returns the results
 * as a Lua table.
 *
 * The function returns three values:
 * - A boolean indicating success (true) or failure (false).
 * - An error message string if the operation failed, or nil if it succeeded.
 * - A Lua table containing the search results if the operation succeeded, or nil if it failed.
 *
 * @param L The Lua state.
 * @return The number of return values on the Lua stack.
 */
int lua_find(lua_State* L) {
    const char* root = luaL_checkstring(L, 1);

    if (!fs::exists(root)) {
        lua_pushboolean(L, 0);  // success = false
        lua_pushstring(L, "Error: Path does not exist");
        lua_pushnil(L); // no results
        return 3; // Return success, error message, and nil results
    }

    if (!fs::is_directory(root)) {
        lua_pushboolean(L, 0);  // success = false
        lua_pushstring(L, "Error: Path is not a directory");
        lua_pushnil(L); // no results
        return 3; // Return success, error message, and nil results
    }

    FindOptions options;
    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "mindepth");
        if (lua_isnumber(L, -1)) options.mindepth = lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "maxdepth");
        if (lua_isnumber(L, -1)) options.maxdepth = lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "type");
        if (lua_isstring(L, -1)) options.type = lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "name");
        if (lua_isstring(L, -1)) options.name = lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "iname");
        if (lua_isstring(L, -1)) options.iname = lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "path");
        if (lua_isstring(L, -1)) options.path = lua_tostring(L, -1);
        lua_pop(L, 1);
    }

    std::vector<fs::path> results;
    try {
        find(root, options, 0, results);
    } catch (const fs::filesystem_error& e) {
        lua_pushboolean(L, 0);  // success = false
        lua_pushstring(L, e.what());
        lua_pushnil(L); // no results
        return 3; // Return success, error message, and nil results
    }

    lua_pushboolean(L, 1);  // success = true
    lua_pushnil(L); // no error message
    lua_newtable(L);
    for (size_t i = 0; i < results.size(); ++i) {
        lua_pushstring(L, results[i].string().c_str());
        lua_rawseti(L, -2, i + 1);
    }

    return 3; // Return success, nil error message, and results table
}
