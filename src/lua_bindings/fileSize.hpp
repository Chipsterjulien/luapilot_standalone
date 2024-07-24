#ifndef FILE_SIZE_HPP
#define FILE_SIZE_HPP

#include <optional>
#include <string>
#include <string_view>
#include <cstdint>
#include <lua.hpp>

/**
 * @brief Check if a file exists and is a regular file, and get its size.
 *
 * @param path The file path to check.
 * @param size Reference to store the size of the file if it exists and is a regular file.
 * @return bool True if the file exists and is a regular file, false otherwise.
 */
bool getFileSize(std::string_view path, uintmax_t& size);

/**
 * @brief Lua binding for getting the size of a file.
 *
 * @param L The Lua state.
 * @return int Number of return values (2: file size and error message or nil).
 */
int lua_fileSize(lua_State* L);

#endif // FILE_SIZE_HPP
