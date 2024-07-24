#include "checksum_utils.hpp"
#include "openssl_utils.hpp"
#include "evp_md_ctx_raii.hpp"
#include <openssl/evp.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <system_error>
#include <cstring>  // Pour strerror

std::optional<std::string> calculate_checksum(const std::string &path, const EVP_MD* md) {
    std::ifstream file(path, std::ifstream::binary);
    if (!file) {
        return std::nullopt;
    }

    EVP_MD_CTX_RAII mdctx;
    if (mdctx.get() == nullptr) {
        return std::nullopt;
    }

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
