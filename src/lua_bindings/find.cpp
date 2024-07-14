#include "find.hpp"
#include <regex>
#include <limits>
#include <system_error>

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
 * @return A string containing an error message if any, or an empty string if successful.
 */
std::string find(const fs::path& root, const FindOptions& options, int depth, std::vector<fs::path>& results) {
    if (!fs::exists(root)) {
        return "Error: Path does not exist: " + root.string();
    }

    if (!fs::is_directory(root)) {
        return "Error: Path is not a directory: " + root.string();
    }

    if (depth > options.maxdepth) return "";

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
                std::string err = find(entry.path(), options, depth + 1, results);
                if (!err.empty()) {
                    return err;
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        return e.what();
    }

    return "";
}

/**
 * @brief Lua binding for the find function.
 *
 * This function is a Lua binding for the find function, allowing it to be called from Lua scripts.
 * It retrieves the search options from the Lua stack, performs the search, and returns the results
 * as a Lua table.
 *
 * @param L The Lua state.
 * @return The number of return values on the Lua stack.
 */
int lua_find(lua_State* L) {
    // Check that there are at least 2 arguments
    int argc = lua_gettop(L);
    if (argc < 2) {
        return luaL_error(L, "Expected at least two arguments");
    }

    // Check that the first argument is a string (root path)
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as the first argument");
    }

    // Check that the second argument is a table (options)
    if (!lua_istable(L, 2)) {
        return luaL_error(L, "Expected a table as the second argument");
    }

    const char* root = luaL_checkstring(L, 1);

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
    std::string error_message = find(root, options, 0, results);

    if (!error_message.empty()) {
        lua_pushnil(L);
        lua_pushstring(L, error_message.c_str());
        return 2; // Return nil and error message
    }

    lua_newtable(L);
    for (size_t i = 0; i < results.size(); ++i) {
        lua_pushstring(L, results[i].string().c_str());
        lua_rawseti(L, -2, i + 1);
    }
    lua_pushnil(L); // No error

    return 2; // Return results table and nil error
}
