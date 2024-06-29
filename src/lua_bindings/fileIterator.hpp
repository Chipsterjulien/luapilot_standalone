#ifndef FILE_ITERATOR_HPP
#define FILE_ITERATOR_HPP

#include <lua.hpp>
#include <filesystem>
#include <vector>
#include <string>

/**
 * @class FileIterator
 * @brief A class to iterate over files in a directory, optionally recursively.
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
     * @return The next file as a string.
     */
    const std::string& next();

    /**
     * @brief Checks if there are more files to iterate over.
     * @return True if there are more files, false otherwise.
     */
    bool hasNext() const;

    /**
     * @brief Checks if the iterator is closed.
     * @return True if the iterator is closed, false otherwise.
     */
    bool isClosed() const;

    /**
     * @brief Sets the closed state of the iterator.
     * @param value The new closed state.
     */
    void setClosed(bool value);

private:
    /**
     * @brief Loads the files from the given path.
     * @param path The directory path to load files from.
     * @param recursive If true, load files recursively from subdirectories.
     */
    void loadFiles(const std::string& path, bool recursive);

    std::vector<std::string> files;
    std::vector<std::string>::iterator current;
    bool closed;
};

extern "C" {
    /**
     * @brief Creates a new FileIterator and pushes it onto the Lua stack.
     * @param L The Lua state.
     * @return The number of return values.
     */
    int lua_createFileIterator(lua_State* L);

    /**
     * @brief Gets the next file from the FileIterator.
     * @param L The Lua state.
     * @return The number of return values.
     */
    int lua_nextFile(lua_State* L);

    /**
     * @brief Garbage collects the FileIterator.
     * @param L The Lua state.
     * @return The number of return values.
     */
    int lua_gcFileIterator(lua_State* L);

    /**
     * @brief Opens the file_iterator module in Lua.
     * @param L The Lua state.
     * @return The number of return values.
     */
    int luaopen_file_iterator(lua_State* L);
}

#endif // FILE_ITERATOR_HPP
