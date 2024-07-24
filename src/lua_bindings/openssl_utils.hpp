#ifndef OPENSSL_UTILS_HPP
#define OPENSSL_UTILS_HPP

#include <string>
#include <openssl/err.h>

/**
 * @brief Retrieves and formats the last OpenSSL error.
 *
 * @return A string containing the last OpenSSL error message.
 */
inline std::string get_openssl_error() {
    unsigned long err_code = ERR_get_error();
    char err_buf[256];
    ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
    return std::string(err_buf);
}

#endif // OPENSSL_UTILS_HPP
