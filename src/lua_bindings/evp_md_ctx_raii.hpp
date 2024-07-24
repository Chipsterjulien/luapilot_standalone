#ifndef EVP_MD_CTX_RAII_HPP
#define EVP_MD_CTX_RAII_HPP

#include <openssl/evp.h>

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
    EVP_MD_CTX* get() const { return ctx; }

private:
    EVP_MD_CTX* ctx;
};

#endif // EVP_MD_CTX_RAII_HPP
