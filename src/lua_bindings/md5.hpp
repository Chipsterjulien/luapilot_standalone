#ifndef MD5_HPP
#define MD5_HPP

#include <openssl/evp.h>
#include <optional>
#include <string>
#include <lua.hpp>

std::optional<std::string> md5sum(const std::string &path);
int lua_md5sum(lua_State *L);

#endif // MD5_HPP