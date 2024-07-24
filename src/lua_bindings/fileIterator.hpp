#ifndef FILEITERATOR_HPP
#define FILEITERATOR_HPP

#include <filesystem>
#include <vector>
#include <string>
#include <optional>
#include <memory>
#include <lua.hpp>

/**
 * @brief Class to iterate over files in a directory.
 */
class FileIterator {
public:
    /**
     * @brief Constructs a FileIterator for the given path.
     * @param path The directory path to iterate over.
     * @param recursive If true, iterate recursively through subdirectories.
     */
    FileIterator(const std::string& path, bool recursive);

    /**
     * @brief Returns the next file in the directory.
     * @return An optional string containing the next file path, or std::nullopt if no more files are available.
     */
    std::optional<std::string> next();

    /**
     * @brief Checks if there are more files to iterate over.
     * @return True if there are more files, false otherwise.
     */
    bool hasNext() const;

private:
    /**
     * @brief Loads the files from the given path.
     * @param path The directory path to load files from.
     * @param recursive If true, load files recursively from subdirectories.
     */
    void loadFiles(const std::filesystem::path& path, bool recursive);

    std::vector<std::string> files; ///< Vector to store file paths.
    std::vector<std::string>::iterator current; ///< Iterator to the current file in the vector.
};

/**
 * @brief Lua binding for getting the next file from the FileIterator.
 * @param L The Lua state.
 * @return The number of return values.
 */
int lua_nextFile(lua_State* L);

/**
 * @brief Lua binding for garbage collecting the FileIterator.
 * @param L The Lua state.
 * @return The number of return values.
 */
int lua_gcFileIterator(lua_State* L);

/**
 * @brief Lua binding for creating a new FileIterator.
 * @param L The Lua state.
 * @return The number of return values.
 */
int lua_createFileIterator(lua_State* L);

/**
 * @brief Creates the metatable for FileIterator.
 * @param L The Lua state.
 * @return The number of return values.
 */
int file_iterator_create_meta(lua_State *L);

/**
 * @brief Opens the file_iterator module in Lua.
 * @param L The Lua state.
 * @return The number of return values.
 */
extern "C" int luaopen_file_iterator(lua_State* L);

#endif // FILEITERATOR_HPP
