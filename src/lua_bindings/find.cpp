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

namespace
{

    struct FindOptions
    {
        int mindepth = 0;
        int maxdepth = std::numeric_limits<int>::max();
        std::string type;
        std::string name;  // ECMAScript regex (e.g. ".*\\.cpp$")
        std::string iname; // idem, case-insensitive
        std::string path;  // ECMAScript regex, searched on the full path
    };

    struct RegexCache
    {
        std::unordered_map<std::string, std::regex> name_regex;
        std::unordered_map<std::string, std::regex> iname_regex;
    };

    // Mono-thread runtime : un cache global est sûr. À déplacer en local
    // si on autorisait un jour plusieurs lua_State concurrents.
    RegexCache cache;

    // Les patterns sont passés tels quels à std::regex, en syntaxe ECMAScript
    // (équivalent à la regex PCRE habituelle : \d, \w, [a-zA-Z], etc.).
    // Pas de pré-traitement type "Lua pattern" : la pseudo-traduction des `%`
    // était un faux-ami qui produisait silencieusement des regex incorrectes
    // (`%a` devenait `\a` = caractère BEL, pas une classe de lettres).
    const std::regex &get_or_compile_regex(RegexCache &cache, const std::string &pattern,
                                           bool case_insensitive = false)
    {
        auto &regex_map = case_insensitive ? cache.iname_regex : cache.name_regex;
        auto it = regex_map.find(pattern);
        if (it == regex_map.end())
        {
            std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript;
            if (case_insensitive)
            {
                flags |= std::regex_constants::icase;
            }
            auto [new_it, _] = regex_map.try_emplace(pattern, std::regex(pattern, flags));
            return new_it->second;
        }
        return it->second;
    }

    bool matches_options(const fs::directory_entry &entry, const FindOptions &options,
                         RegexCache &cache)
    {
        if (!options.type.empty())
        {
            if ((options.type == "f" && !fs::is_regular_file(entry)) ||
                (options.type == "d" && !fs::is_directory(entry)))
            {
                return false;
            }
        }

        if (!options.name.empty())
        {
            const auto &name_regex = get_or_compile_regex(cache, options.name);
            if (!std::regex_match(entry.path().filename().string(), name_regex))
            {
                return false;
            }
        }

        if (!options.iname.empty())
        {
            const auto &iname_regex = get_or_compile_regex(cache, options.iname, true);
            if (!std::regex_match(entry.path().filename().string(), iname_regex))
            {
                return false;
            }
        }

        if (!options.path.empty())
        {
            // Note : pas de cache pour `path` (peu de répétition attendue,
            // contrairement à `name` qui filtre des milliers d'entrées).
            const std::regex path_regex(options.path, std::regex_constants::ECMAScript);
            if (!std::regex_search(entry.path().string(), path_regex))
            {
                return false;
            }
        }

        return true;
    }

    FindOptions parse_options(lua_State *L, int index)
    {
        FindOptions options;

        lua_getfield(L, index, "mindepth");
        if (lua_isnumber(L, -1))
        {
            options.mindepth = lua_tointeger(L, -1);
        }
        else if (!lua_isnil(L, -1))
        {
            luaL_error(L, "Expected number for mindepth");
        }
        lua_pop(L, 1);

        lua_getfield(L, index, "maxdepth");
        if (lua_isnumber(L, -1))
        {
            options.maxdepth = lua_tointeger(L, -1);
        }
        else if (!lua_isnil(L, -1))
        {
            luaL_error(L, "Expected number for maxdepth");
        }
        lua_pop(L, 1);

        lua_getfield(L, index, "type");
        if (lua_isstring(L, -1))
        {
            options.type = lua_tostring(L, -1);
        }
        else if (!lua_isnil(L, -1))
        {
            luaL_error(L, "Expected string for type");
        }
        lua_pop(L, 1);

        lua_getfield(L, index, "name");
        if (lua_isstring(L, -1))
        {
            options.name = lua_tostring(L, -1);
        }
        else if (!lua_isnil(L, -1))
        {
            luaL_error(L, "Expected string for name");
        }
        lua_pop(L, 1);

        lua_getfield(L, index, "iname");
        if (lua_isstring(L, -1))
        {
            options.iname = lua_tostring(L, -1);
        }
        else if (!lua_isnil(L, -1))
        {
            luaL_error(L, "Expected string for iname");
        }
        lua_pop(L, 1);

        lua_getfield(L, index, "path");
        if (lua_isstring(L, -1))
        {
            options.path = lua_tostring(L, -1);
        }
        else if (!lua_isnil(L, -1))
        {
            luaL_error(L, "Expected string for path");
        }
        lua_pop(L, 1);

        return options;
    }

    std::optional<std::string> find(const fs::path &root, const FindOptions &options,
                                    std::function<void(const fs::path &)> callback,
                                    RegexCache &cache)
    {
        if (!fs::exists(root))
        {
            return "path does not exist: " + root.string();
        }

        if (!fs::is_directory(root))
        {
            return "path is not a directory: " + root.string();
        }

        try
        {
            for (auto it = fs::recursive_directory_iterator(root);
                 it != fs::recursive_directory_iterator(); ++it)
            {
                int depth = it.depth();
                if (depth < options.mindepth || depth > options.maxdepth)
                {
                    if (depth > options.maxdepth)
                    {
                        it.pop();
                    }
                    continue;
                }

                if (matches_options(*it, options, cache))
                {
                    callback(it->path());
                }
            }
        }
        catch (const std::exception &e)
        {
            return std::string(e.what());
        }
        catch (...)
        {
            return "unknown error";
        }

        return std::nullopt;
    }

} // namespace

int lua_find(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc < 2)
    {
        return luaL_error(L, "Expected at least two arguments");
    }

    if (!lua_isstring(L, 1))
    {
        return luaL_error(L, "Expected a string as the first argument");
    }

    if (!lua_istable(L, 2))
    {
        return luaL_error(L, "Expected a table as the second argument");
    }

    const char *root = luaL_checkstring(L, 1);
    FindOptions options = parse_options(L, 2);

    lua_newtable(L);
    int result_index = lua_gettop(L);

    int file_index = 1;

    auto callback = [L, result_index, &file_index](const fs::path &path)
    {
        lua_pushstring(L, path.string().c_str());
        lua_rawseti(L, result_index, file_index++);
    };

    if (auto error_message = find(root, options, callback, cache))
    {
        lua_pushnil(L);
        lua_pushstring(L, error_message->c_str());
        return 2;
    }

    lua_pushnil(L);
    return 2;
}
