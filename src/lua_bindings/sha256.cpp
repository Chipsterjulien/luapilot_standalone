#include "sha256.hpp"
#include <openssl/evp.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <system_error>

/**
 * @brief Calculates the SHA-256 checksum of a file.
 *
 * @param path The path to the file.
 * @return A tuple containing the SHA-256 checksum as a hexadecimal string and an error message if any.
 */
std::tuple<std::string, std::string> sha256sum(const std::string &path) {
    std::ifstream file(path, std::ifstream::binary);
    if (!file) {
        return {"", std::make_error_code(std::errc::no_such_file_or_directory).message()};
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == nullptr) {
        return {"", std::make_error_code(std::errc::not_enough_memory).message()};
    }

    const EVP_MD *md = EVP_sha256();
    if (EVP_DigestInit_ex(mdctx, md, nullptr) != 1) {
        EVP_MD_CTX_free(mdctx);
        return {"", "Error initializing digest"};
    }

    char buffer[4096];
    while (file.good()) {
        file.read(buffer, sizeof(buffer));
        if (EVP_DigestUpdate(mdctx, buffer, file.gcount()) != 1) {
            EVP_MD_CTX_free(mdctx);
            return {"", "Error updating digest"};
        }
    }

    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int result_len;
    if (EVP_DigestFinal_ex(mdctx, result, &result_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        return {"", "Error finalizing digest"};
    }

    EVP_MD_CTX_free(mdctx);

    std::ostringstream hexStream;
    hexStream << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < result_len; ++i) {
        hexStream << std::setw(2) << (int)result[i];
    }

    return {hexStream.str(), ""};
}

/**
 * @brief Lua binding for the sha256sum function.
 *
 * @param L The Lua state.
 * @return 2 values: the SHA-256 checksum or nil, and an error message or nil.
 */
int lua_sha256sum(lua_State *L) {
    // Check if there is one argument passed
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check that the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    const char *path = luaL_checkstring(L, 1);
    auto [sha256sum_str, err] = sha256sum(path);

    if (!err.empty()) {
        lua_pushnil(L);
        lua_pushstring(L, err.c_str());
        return 2;
    }

    lua_pushstring(L, sha256sum_str.c_str());
    lua_pushnil(L);
    return 2;
}
