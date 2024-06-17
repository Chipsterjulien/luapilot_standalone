# Définit la version minimale de CMake requise
cmake_minimum_required(VERSION 3.10)

# Nom du projet
project(luapilot)

# Spécifie le standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Chemins des bibliothèques et des en-têtes
set(LUA_INCLUDE_DIR "${LUA_INCLUDE}")
set(LUA_LIBRARIES "${LUA_LIB}")

set(LIBZIP_INCLUDE_DIR "${LIBZIP_INCLUDE}")
set(LIBZIP_LIBRARIES "${LIBZIP_LIB}")

set(PHYSFS_INCLUDE_DIR "${PHYSFS_INCLUDE}")
set(PHYSFS_LIBRARIES "${PHYSFS_LIB}")

set(BZIP2_LIBRARIES "${BZIP2_LIB}")

set(OPENSSL_LIBRARIES "${OPENSSL_LIB}")
set(CRYPTO_LIBRARIES "${CRYPTO_LIB}")

set(LIBZ_LIBRARIES "${LIBZ_LIB}")

# Vérifiez que les chemins sont définis
if(NOT LUA_INCLUDE_DIR OR NOT LUA_LIBRARIES)
    message(FATAL_ERROR "Lua include directory or library path is not set")
endif()

if(NOT LIBZIP_INCLUDE_DIR OR NOT LIBZIP_LIBRARIES)
    message(FATAL_ERROR "libzip include directory or library path is not set")
endif()

if(NOT PHYSFS_INCLUDE_DIR OR NOT PHYSFS_LIBRARIES)
    message(FATAL_ERROR "PhysFS include directory or library path is not set")
endif()

if(NOT BZIP2_LIBRARIES)
    message(FATAL_ERROR "bzip2 library is not set")
endif()

if(NOT OPENSSL_LIBRARIES)
    message(FATAL_ERROR "openssl library is not set")
endif()

if(NOT CRYPTO_LIBRARIES)
    message(FATAL_ERROR "openssl crypto library is not set")
endif()

if(NOT LIBZ_LIBRARIES)
    message(FATAL_ERROR "libz library is not set")
endif()

# Afficher les valeurs des variables pour le débogage
message(STATUS "LUA_INCLUDE_DIR: ${LUA_INCLUDE_DIR}")
message(STATUS "LUA_LIBRARIES: ${LUA_LIBRARIES}")
message(STATUS "LIBZIP_INCLUDE_DIR: ${LIBZIP_INCLUDE_DIR}")
message(STATUS "LIBZIP_LIBRARIES: ${LIBZIP_LIBRARIES}")
message(STATUS "PHYSFS_INCLUDE_DIR: ${PHYSFS_INCLUDE_DIR}")
message(STATUS "PHYSFS_LIBRARIES: ${PHYSFS_LIBRARIES}")
message(STATUS "BZIP2_LIBRARIES: ${BZIP2_LIBRARIES}")
message(STATUS "OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
message(STATUS "LIBZ_LIBRARIES: ${LIBZ_LIBRARIES}")

# Ajouter les répertoires d'inclusion uniquement si les chemins ne sont pas vides
if (LUA_INCLUDE_DIR)
    include_directories("${LUA_INCLUDE_DIR}")
endif()

if (LIBZIP_INCLUDE_DIR)
    include_directories("${LIBZIP_INCLUDE_DIR}")
endif()

if (PHYSFS_INCLUDE_DIR)
    include_directories("${PHYSFS_INCLUDE_DIR}")
endif()

# if (BZIP2_LIBRARIES)
#     include_directories("${BZIP2_LIBRARIES}")
# endif()

# if (OPENSSL_LIBRARIES)
#     include_directories("${OPENSSL_LIBRARIES}")
# endif()


# if (LIBZIP_LIBRARIES)
#     include_directories("${LIBZ_LIBRARIES}")
# endif()

# Ajouter les répertoires d'inclusion pour le projet
include_directories("${PROJECT_SOURCE_DIR}/src")

# Rechercher récursivement tous les fichiers .cpp et .hpp
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")

# Afficher les fichiers sources et les en-têtes trouvés pour le débogage
message(STATUS "SOURCES: ${SOURCES}")
message(STATUS "HEADERS: ${HEADERS}")

# Spécifier les fichiers sources et les en-têtes et créer l'exécutable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Lier les bibliothèques et ajouter les bibliothèques système nécessaires
# target_link_libraries(${PROJECT_NAME} "${LUA_LIBRARIES}" "${LIBZIP_LIBRARIES}" "${PHYSFS_LIBRARIES}" dl z bz2 ssl crypto -static-libgcc -static-libstdc++ -lz -lbz2 -pthread)
# target_link_libraries(${PROJECT_NAME} "${LUA_LIBRARIES}" "${LIBZIP_LIBRARIES}" "${PHYSFS_LIBRARIES}" dl z bz2 ssl crypto)
target_link_libraries(${PROJECT_NAME}
    "${LUA_LIBRARIES}"
    "${LIBZIP_LIBRARIES}"
    "${PHYSFS_LIBRARIES}"
    "${BZIP2_LIBRARIES}"
    "${OPENSSL_LIBRARIES}"
    "${CRYPTO_LIBRARIES}"
    "${LIBZ_LIBRARIES}"
    dl
    pthread
)


# Spécifier les options pour créer un binaire statique
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_SEARCH_END_STATIC 1)

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-static")
# set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-static -static-libgcc -static-libstdc++")




# cmake_minimum_required(VERSION 3.10)
# project(luapilot)

# set(CMAKE_CXX_STANDARD 17)

# # Trouver les packages nécessaires
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(ZIP REQUIRED libzip)
# pkg_check_modules(PHYSFS REQUIRED physfs)

# # Utiliser OPENSSL_ROOT_DIR si spécifié
# if (DEFINED OPENSSL_ROOT_DIR)
#     set(OPENSSL_USE_STATIC_LIBS TRUE)
#     set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
#     set(OPENSSL_LIBRARIES ${OPENSSL_ROOT_DIR}/lib)
#     find_package(OpenSSL REQUIRED)
# else()
#     find_package(OpenSSL REQUIRED)
# endif()

# # Inclure les répertoires d'en-têtes
# include_directories(${CMAKE_SOURCE_DIR}/external/lua/include)
# include_directories(${CMAKE_SOURCE_DIR}/include)
# include_directories(${OPENSSL_INCLUDE_DIR})
# include_directories(src)

# # Ajouter les répertoires d'inclusion et les bibliothèques Lua
# include_directories(${LUA_INCLUDE_DIR})
# link_directories(${LUA_LIBRARY})
# link_directories(${CMAKE_SOURCE_DIR}/libs)

# # Rechercher récursivement tous les fichiers source dans src/ et ses sous-répertoires
# file(GLOB_RECURSE SOURCES "src/*.cpp")

# # Ajouter la bibliothèque Lua statique
# add_library(lua STATIC IMPORTED)
# set_target_properties(lua PROPERTIES
#     IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/external/lua/lib/liblua.a"
#     INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/external/lua/include"
# )

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -s")
# # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -Wl,--gc-sections")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

# # Créer l'exécutable
# add_executable(luapilot ${SOURCES})

# # Lier les bibliothèques statiques
# target_link_libraries(luapilot lua
#     ${CMAKE_SOURCE_DIR}/libs/libzip.a
#     ${CMAKE_SOURCE_DIR}/libs/libphysfs.a
#     ${OPENSSL_LIBRARIES}
#     -lz
#     -lbz2
#     -pthread
#     -static-libgcc
#     -static-libstdc++
# )
