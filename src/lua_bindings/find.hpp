#ifndef FIND_HPP
#define FIND_HPP

#include <vector>
#include <string>
#include <filesystem>
#include <lua.hpp>

namespace fs = std::filesystem;

/**
 * @brief Options for the find function to control search behavior.
 *
 * This struct contains various options that control the behavior of the find function,
 * such as depth limits, type filtering, name matching, and path matching.
 */
struct FindOptions {
    int mindepth = 0;  /**< Minimum depth for search. */
    int maxdepth = std::numeric_limits<int>::max();  /**< Maximum depth for search. */
    std::string type;  /**< File type filter (e.g., "f" for files, "d" for directories). */
    std::string name;  /**< Regular expression for matching file names. */
    std::string iname;  /**< Case-insensitive regular expression for matching file names. */
    std::string path;  /**< Regular expression for matching file paths. */
};

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
std::string find(const fs::path& root, const FindOptions& options, int depth, std::vector<fs::path>& results);

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
int lua_find(lua_State* L);

#endif // FIND_HPP
