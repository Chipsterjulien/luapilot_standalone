#ifndef EVP_MD_CTX_RAII_HPP
#define EVP_MD_CTX_RAII_HPP

#include <openssl/evp.h>

/**
 * @brief RAII wrapper for OpenSSL's EVP_MD_CTX*.
 *
 * Allocates the context at construction and frees it at destruction,
 * so callers never have to remember EVP_MD_CTX_free even when an error
 * causes an early return.
 *
 * Non-copyable : un contexte EVP_MD_CTX appartient à exactement un
 * wrapper. Autoriser la copie produirait un double-free (les deux copies
 * appelleraient EVP_MD_CTX_free sur le même pointeur).
 */
class EVP_MD_CTX_RAII
{
public:
    EVP_MD_CTX_RAII() : ctx(EVP_MD_CTX_new()) {}
    ~EVP_MD_CTX_RAII()
    {
        if (ctx)
        {
            EVP_MD_CTX_free(ctx);
        }
    }

    // Non copyable, non assignable : voir commentaire de classe.
    EVP_MD_CTX_RAII(const EVP_MD_CTX_RAII &) = delete;
    EVP_MD_CTX_RAII &operator=(const EVP_MD_CTX_RAII &) = delete;

    EVP_MD_CTX *get() const { return ctx; }

private:
    EVP_MD_CTX *ctx;
};

#endif // EVP_MD_CTX_RAII_HPP