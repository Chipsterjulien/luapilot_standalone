cmake_minimum_required(VERSION 3.10)
project(luapilot)

set(CMAKE_CXX_STANDARD 17)

# Trouver les packages nécessaires
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZIP REQUIRED libzip)
pkg_check_modules(PHYSFS REQUIRED physfs)

# Utiliser OPENSSL_ROOT_DIR si spécifié
if (DEFINED OPENSSL_ROOT_DIR)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
    set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
    set(OPENSSL_LIBRARIES ${OPENSSL_ROOT_DIR}/lib)
    find_package(OpenSSL REQUIRED)
else()
    find_package(OpenSSL REQUIRED)
endif()

# Inclure les répertoires d'en-têtes
include_directories(${CMAKE_SOURCE_DIR}/external/lua/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(src)

# Ajouter les répertoires d'inclusion et les bibliothèques Lua
include_directories(${LUA_INCLUDE_DIR})
link_directories(${LUA_LIBRARY})
link_directories(${CMAKE_SOURCE_DIR}/libs)

# Rechercher récursivement tous les fichiers source dans src/ et ses sous-répertoires
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Ajouter la bibliothèque Lua statique
add_library(lua STATIC IMPORTED)
set_target_properties(lua PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/external/lua/lib/liblua.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/external/lua/include"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -s")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

# Créer l'exécutable
add_executable(luapilot ${SOURCES})

# Lier les bibliothèques statiques
target_link_libraries(luapilot lua
    ${CMAKE_SOURCE_DIR}/libs/libzip.a
    ${CMAKE_SOURCE_DIR}/libs/libphysfs.a
    ${OPENSSL_LIBRARIES}
    -lz
    -lbz2
    -pthread
    -static-libgcc
    -static-libstdc++
)
