#include "md5.hpp"
#include <openssl/evp.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <system_error>
#include <cstring>  // Pour strerror

/**
 * @brief RAII class for EVP_MD_CTX.
 */
class EVP_MD_CTX_RAII {
public:
    EVP_MD_CTX_RAII() : ctx(EVP_MD_CTX_new()) {}
    ~EVP_MD_CTX_RAII() {
        if (ctx) {
            EVP_MD_CTX_free(ctx);
        }
    }
    EVP_MD_CTX* get() const { return ctx; }  // Ajout de const

private:
    EVP_MD_CTX* ctx;
};

/**
 * @brief Calculates the MD5 checksum of a file.
 *
 * @param path The path to the file.
 * @return A tuple containing the MD5 checksum as a hexadecimal string and an error message if any.
 */
std::tuple<std::string, std::string> md5sum(const std::string &path) {
    std::ifstream file(path, std::ifstream::binary);
    if (!file) {
        return {"", std::strerror(errno)};
    }

    EVP_MD_CTX_RAII mdctx;
    if (mdctx.get() == nullptr) {
        return {"", "Error creating context"};
    }

    const EVP_MD* md = EVP_md5();
    if (EVP_DigestInit_ex(mdctx.get(), md, nullptr) != 1) {
        return {"", "Error initializing digest"};
    }

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {  // Condition améliorée
        if (EVP_DigestUpdate(mdctx.get(), buffer, file.gcount()) != 1) {
            return {"", "Error updating digest"};
        }
    }

    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int result_len;
    if (EVP_DigestFinal_ex(mdctx.get(), result, &result_len) != 1) {
        return {"", "Error finalizing digest"};
    }

    std::ostringstream hexStream;
    hexStream << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < result_len; ++i) {
        hexStream << std::setw(2) << static_cast<int>(result[i]);
    }

    return {hexStream.str(), ""};
}

/**
 * @brief Lua binding for the md5sum function.
 *
 * @param L The Lua state.
 * @return 2 values: the MD5 checksum or nil, and an error message or nil.
 */
int lua_md5sum(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    // Check that the argument is a string
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    const char* path = luaL_checkstring(L, 1);
    auto [md5sum_str, err] = md5sum(path);

    if (!err.empty()) {
        lua_pushnil(L);
        lua_pushstring(L, err.c_str());
        return 2;
    }

    lua_pushstring(L, md5sum_str.c_str());
    lua_pushnil(L);
    return 2;
}
