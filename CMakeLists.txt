cmake_minimum_required(VERSION 3.16)
project(luapilot LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)

# External libs provided by build_local.sh
if(NOT LUA_INCLUDE OR NOT LUA_LIB)
    message(FATAL_ERROR "Lua paths not set (LUA_INCLUDE, LUA_LIB)")
endif()

if(NOT OPENSSL_LIB OR NOT CRYPTO_LIB)
    message(FATAL_ERROR "OpenSSL paths not set (OPENSSL_LIB, CRYPTO_LIB)")
endif()

if(NOT MINIZ_SRC OR NOT MINIZ_INCLUDE)
    message(FATAL_ERROR "miniz paths not set (MINIZ_SRC, MINIZ_INCLUDE)")
endif()

# nlohmann/json : header-only, fourni par build_local.sh.
if(NOT JSON_INCLUDE)
    message(FATAL_ERROR "JSON_INCLUDE not set (nlohmann/json header dir)")
endif()

# cpp-httplib : header-only, fourni par build_local.sh. TLS activé
# localement dans src/lua_bindings/http.cpp (CPPHTTPLIB_OPENSSL_SUPPORT),
# pas en define global ici : httplib.h n'est inclus que dans cette TU.
if(NOT HTTPLIB_INCLUDE)
    message(FATAL_ERROR "HTTPLIB_INCLUDE not set (cpp-httplib header dir)")
endif()

# toml++ : header-only, fourni par build_local.sh. C++17 minimum
# satisfait (projet en C++23). Pas de TLS, pas de threads.
if(NOT TOMLPP_INCLUDE)
    message(FATAL_ERROR "TOMLPP_INCLUDE not set (toml++ header dir)")
endif()

# Dossier des headers générés (embedded_inspect.hpp et co)
# Doit être créé et peuplé par build_local.sh AVANT le cmake.
if(NOT GENERATED_INCLUDE)
    message(FATAL_ERROR "GENERATED_INCLUDE not set (build/generated path)")
endif()

# Sources
file(GLOB_RECURSE PROJECT_SOURCES
    src/lua_bindings/*.cpp
    src/project_core/*.cpp
)
list(APPEND PROJECT_SOURCES
    src/main.cpp
    ${MINIZ_SRC}
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${LUA_INCLUDE}
    ${MINIZ_INCLUDE}
    ${JSON_INCLUDE}
    ${HTTPLIB_INCLUDE}
    ${TOMLPP_INCLUDE}
    ${GENERATED_INCLUDE}
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LUA_LIB}
    ${OPENSSL_LIB}
    ${CRYPTO_LIB}
    dl
    pthread
)

# libatomic : nécessaire sur les architectures 32-bit (ARMv6 / ARMv7 /
# i386) qui n'ont pas d'instructions atomiques 64-bit en hardware.
# OpenSSL 3.x utilise des __atomic_load_8 / __atomic_fetch_or_8 dans
# son crypto/threads_pthread.c, ce qui déclenche des appels à des
# builtins software fournis par libatomic. Sans cette lib au link,
# on a des erreurs comme :
# référence indéfinie vers « __atomic_is_lock_free »
# référence indéfinie vers « __atomic_load_8 »
#
# CMAKE_SIZEOF_VOID_P EQUAL 4 = plateforme 32-bit. On laisse le linker
# (GCC/ld) résoudre -latomic lui-même : il connaît les paths
# multi-arch Debian (/usr/lib/arm-linux-gnueabihf/, etc.) que
# find_library peut rater. Sur les systèmes Linux modernes, libatomic
# est fournie par gcc-runtime, donc présente partout où gcc est
# installé.
#
# Sur x86_64 et aarch64 (64-bit), le CPU fait l'atomique 64-bit
# nativement, pas besoin de cette lib.
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_libraries(${PROJECT_NAME} PRIVATE atomic)
    message(STATUS "32-bit platform detected, linking libatomic")
endif()

target_link_options(${PROJECT_NAME} PRIVATE
    -static-libgcc
    -static-libstdc++
)
