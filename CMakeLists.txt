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

target_link_options(${PROJECT_NAME} PRIVATE
    -static-libgcc
    -static-libstdc++
)
