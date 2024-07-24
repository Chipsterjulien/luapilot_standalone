#include "find.hpp"
#include <regex>
#include <limits>
#include <system_error>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <optional>
#include <functional>
#include <unordered_map>

namespace fs = std::filesystem;

struct RegexCache {
    std::unordered_map<std::string, std::regex> name_regex;
    std::unordered_map<std::string, std::regex> iname_regex;
};

static RegexCache cache;

/**
 * @brief Transforms Lua regex escape sequences to C++ regex escape sequences.
 *
 * This function takes a Lua regex pattern string and converts any Lua-specific
 * escape sequences (e.g., %.) to their C++ regex equivalents (e.g., \.).
 *
 * @param lua_regex The Lua regex pattern string.
 * @return The transformed C++ regex pattern string.
 */
std::string transform_lua_regex(const std::string_view lua_regex) {
    std::string cpp_regex;
    cpp_regex.reserve(lua_regex.size());

    for (size_t i = 0; i < lua_regex.size(); ++i) {
        if (lua_regex[i] == '%' && (i + 1 < lua_regex.size() && lua_regex[i + 1] == '%')) {
            cpp_regex.push_back('%');
            ++i; // Skip the next '%'
        } else if (lua_regex[i] == '%') {
            cpp_regex.push_back('\\');
        } else {
            cpp_regex.push_back(lua_regex[i]);
        }
    }

    return cpp_regex;
}

/**
 * @brief Gets or compiles a regex pattern.
 *
 * This function retrieves a precompiled regex pattern from the cache or compiles it if not found.
 *
 * @param cache The regex cache.
 * @param pattern The regex pattern string.
 * @param case_insensitive Whether the regex should be case-insensitive.
 * @return The compiled regex pattern.
 */
std::regex get_or_compile_regex(RegexCache& cache, const std::string& pattern, bool case_insensitive = false) {
    auto& regex_map = case_insensitive ? cache.iname_regex : cache.name_regex;
    auto it = regex_map.find(pattern);
    if (it == regex_map.end()) {
        std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript;
        if (case_insensitive) {
            flags |= std::regex_constants::icase;
        }
        auto transformed_pattern = transform_lua_regex(pattern);
        auto [new_it, success] = regex_map.try_emplace(pattern, std::regex(transformed_pattern, flags));
        return new_it->second;
    }
    return it->second;
}

/**
 * @brief Checks if a directory entry matches the given options.
 *
 * This function checks if a directory entry matches the specified search options.
 *
 * @param entry The directory entry to check.
 * @param options The search options.
 * @param cache The regex cache.
 * @return True if the entry matches the options, false otherwise.
 */
bool matches_options(const fs::directory_entry& entry, const FindOptions& options, RegexCache& cache) {
    // Filter by type (file or directory)
    if (!options.type.empty()) {
        if ((options.type == "f" && !fs::is_regular_file(entry)) ||
            (options.type == "d" && !fs::is_directory(entry))) {
            return false;
        }
    }

    // Filter by name using case-sensitive regex
    if (!options.name.empty()) {
        const auto& name_regex = get_or_compile_regex(cache, options.name);
        if (!std::regex_match(entry.path().filename().string(), name_regex)) {
            return false;
        }
    }

    // Filter by name using case-insensitive regex
    if (!options.iname.empty()) {
        const auto& iname_regex = get_or_compile_regex(cache, options.iname, true);
        if (!std::regex_match(entry.path().filename().string(), iname_regex)) {
            return false;
        }
    }

    // Filter by path using regex search (more flexible than match)
    if (!options.path.empty()) {
        const std::regex path_regex(transform_lua_regex(options.path));
        if (!std::regex_search(entry.path().string(), path_regex)) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Parses the search options from a Lua table.
 *
 * This function reads the search options from the Lua stack and populates the FindOptions structure.
 *
 * @param L The Lua state.
 * @param index The stack index of the options table.
 * @return The populated FindOptions structure.
 */
FindOptions parse_options(lua_State* L, int index) {
    FindOptions options;

    lua_getfield(L, index, "mindepth");
    if (lua_isnumber(L, -1)) {
        options.mindepth = lua_tointeger(L, -1);
    } else if (!lua_isnil(L, -1)) {
        luaL_error(L, "Expected number for mindepth");
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "maxdepth");
    if (lua_isnumber(L, -1)) {
        options.maxdepth = lua_tointeger(L, -1);
    } else if (!lua_isnil(L, -1)) {
        luaL_error(L, "Expected number for maxdepth");
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "type");
    if (lua_isstring(L, -1)) {
        options.type = lua_tostring(L, -1);
    } else if (!lua_isnil(L, -1)) {
        luaL_error(L, "Expected string for type");
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "name");
    if (lua_isstring(L, -1)) {
        options.name = lua_tostring(L, -1);
    } else if (!lua_isnil(L, -1)) {
        luaL_error(L, "Expected string for name");
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "iname");
    if (lua_isstring(L, -1)) {
        options.iname = lua_tostring(L, -1);
    } else if (!lua_isnil(L, -1)) {
        luaL_error(L, "Expected string for iname");
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "path");
    if (lua_isstring(L, -1)) {
        options.path = lua_tostring(L, -1);
    } else if (!lua_isnil(L, -1)) {
        luaL_error(L, "Expected string for path");
    }
    lua_pop(L, 1);

    return options;
}

/**
 * @brief Recursively searches for files and directories based on the provided options.
 *
 * This function performs a recursive search starting from the given root directory,
 * applying the specified options to filter the results. The callback function is called
 * for each file found.
 *
 * @param root The root directory to start the search from.
 * @param options The search options, including filters and depth limits.
 * @param callback The callback function to call for each file found.
 * @param cache The regex cache.
 * @return An optional string containing an error message if any, or an empty optional if successful.
 */
std::optional<std::string> find(const fs::path& root, const FindOptions& options,
                                std::function<void(const fs::path&)> callback, RegexCache& cache) {
    if (!fs::exists(root)) {
        return "Error: Path does not exist: " + root.string();
    }

    if (!fs::is_directory(root)) {
        return "Error: Path is not a directory: " + root.string();
    }

    try {
        for (auto it = fs::recursive_directory_iterator(root); it != fs::recursive_directory_iterator(); ++it) {
            int depth = it.depth();
            if (depth < options.mindepth || depth > options.maxdepth) {
                if (depth > options.maxdepth) {
                    it.pop();
                }
                continue;
            }

            if (matches_options(*it, options, cache)) {
                callback(it->path());
            }
        }
    } catch (const std::exception& e) {
        return "Error: " + std::string(e.what());
    } catch (...) {
        return "An unknown error occurred.";
    }

    return std::nullopt;
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
    FindOptions options = parse_options(L, 2);

    lua_newtable(L);
    int result_index = lua_gettop(L);

    int file_index = 1;

    auto callback = [L, result_index, &file_index](const fs::path& path) {
        lua_pushstring(L, path.string().c_str());
        lua_rawseti(L, result_index, file_index++);
    };

    if (auto error_message = find(root, options, callback, cache)) {
        lua_pushnil(L);
        lua_pushstring(L, error_message->c_str());
        return 2; // Return nil and error message
    }

    lua_pushnil(L); // No error
    return 2; // Return results table and nil error
}
