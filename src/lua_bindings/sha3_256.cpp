#include "sha3_256.hpp"
#include "openssl_utils.hpp"
#include "evp_md_ctx_raii.hpp"
#include <openssl/evp.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <system_error>
#include <cstring>  // Pour strerror

/**
 * @brief Calculates the SHA-3 256 checksum of a file.
 *
 * @param path The path to the file.
 * @return An optional string containing the SHA-3 256 checksum, ou nullopt si une erreur s'est produite.
 */
std::optional<std::string> sha3_256sum(const std::string &path) {
    std::ifstream file(path, std::ifstream::binary);
    if (!file) {
        return std::nullopt;
    }

    EVP_MD_CTX_RAII mdctx;
    if (mdctx.get() == nullptr) {
        return std::nullopt;
    }

    const EVP_MD* md = EVP_sha3_256();
    if (EVP_DigestInit_ex(mdctx.get(), md, nullptr) != 1) {
        return std::nullopt;
    }

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer))) {
        if (EVP_DigestUpdate(mdctx.get(), buffer, file.gcount()) != 1) {
            return std::nullopt;
        }
    }
    if (file.gcount() > 0) {
        if (EVP_DigestUpdate(mdctx.get(), buffer, file.gcount()) != 1) {
            return std::nullopt;
        }
    }

    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int result_len;
    if (EVP_DigestFinal_ex(mdctx.get(), result, &result_len) != 1) {
        return std::nullopt;
    }

    std::ostringstream hexStream;
    hexStream << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < result_len; ++i) {
        hexStream << std::setw(2) << static_cast<int>(result[i]);
    }

    return hexStream.str();
}

/**
 * @brief Lua binding for the sha3_256sum function.
 *
 * @param L The Lua state.
 * @return 2 values: the SHA-3 256 checksum or nil, and an error message or nil.
 */
int lua_sha3_256sum(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "Expected one argument");
    }

    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as argument");
    }

    const char* path = luaL_checkstring(L, 1);
    auto result = sha3_256sum(path);

    if (!result.has_value()) {
        lua_pushnil(L);
        lua_pushstring(L, "Error calculating SHA-3 256 checksum");
        return 2;
    }

    lua_pushstring(L, result->c_str());
    lua_pushnil(L);
    return 2;
}
