#!/bin/bash

# Vérifier si CMake est installé
if ! command -v cmake &> /dev/null; then
    echo "CMake n'est pas installé. Veuillez l'installer avant de continuer."
    exit 1
fi

# Vérifier si pkg-config est installé
if ! command -v pkg-config &> /dev/null; then
    echo "pkg-config n'est pas installé. Veuillez l'installer avant de continuer."
    exit 1
fi

# Vérifier si wget est installé
if ! command -v wget &> /dev/null; then
    echo "wget n'est pas installé. Veuillez l'installer avant de continuer."
    exit 1
fi

# Vérifier que xargs est installé
if ! command -v xargs &> /dev/null; then
    echo "xargs n'est pas installé. Veuillez l'installer avant de continuer."
fi

# Variables
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
DOWNLOAD_DIR="${SCRIPT_DIR}/downloads"
#
PROJECT_BUILD_DIR="${BUILD_DIR}/project_build"
PROJECT_NAME="luapilot"
#
LUA_VERSION="5.4.7"
LUA_DIR="lua-$LUA_VERSION"
LUA_TAR="$LUA_DIR.tar.gz"
LUA_BUILD_DIR="${BUILD_DIR}/lua_build"
LUA_URL="http://www.lua.org/ftp/$LUA_TAR"
LUA_LIB_NAME="liblua.a"
LUA_LIB="${LUA_BUILD_DIR}/${LUA_DIR}/src/${LUA_LIB_NAME}"
LUA_INCLUDE="${LUA_BUILD_DIR}/${LUA_DIR}/src"
#
LIBZIP_VERSION="1.10.1"
LIBZIP_DIR="libzip-$LIBZIP_VERSION"
LIBZIP_TAR="$LIBZIP_DIR.tar.gz"
LIBZIP_URL="https://libzip.org/download/$LIBZIP_TAR"
LIBZIP_BUILD_DIR="${BUILD_DIR}/libzip"
LIBZIP_LIB_NAME="libzip.a"
LIBZIP_HEADER_NAME="zip.h"
LIBZIP_INCLUDE="${LIBZIP_BUILD_DIR}/${LIBZIP_DIR}/lib"
#
PHYSFS_VERSION="3.0.2"
PHYSFS_DIR="physfs-$PHYSFS_VERSION"
PHYSFS_TAR="$PHYSFS_DIR.tar.bz2"
PHYSFS_URL="https://icculus.org/physfs/downloads/$PHYSFS_TAR"
PHYSFS_BUILD_DIR="${BUILD_DIR}/physfs"
PHYSFS_LIB_NAME="libphysfs.a"
PHYSFS_HEADER_NAME="physfs.h"
PHYSFS_INCLUDE="${PHYSFS_BUILD_DIR}/${PHYSFS_DIR}/src"
#
BZIP2_VERSION="1.0.6"
BZIP2_DIR="bzip2-$BZIP2_VERSION"
BZIP2_TAR="$BZIP2_DIR.tar.gz"
BZIP2_BUILD_DIR="${BUILD_DIR}/bzip2"
#
OPENSSL_VERSION="3.3.1"
OPENSSL_DIR="openssl-${OPENSSL_VERSION}"
OPENSSL_TAR="${OPENSSL_DIR}.tar.gz"
OPENSSL_BUILD_DIR="${BUILD_DIR}/openssl"


# Créer les répertoires nécessaires
mkdir -p "$DOWNLOAD_DIR"
mkdir -p "$LUA_BUILD_DIR"
mkdir -p "$BUILD_DIR"


# Télécharger Lua si nécessaire
if [ ! -f "$DOWNLOAD_DIR/$LUA_TAR" ]; then
    echo "Téléchargement de Lua $LUA_VERSION..."
    wget "${LUA_URL}" -O "$DOWNLOAD_DIR/$LUA_TAR"
    if [ $? -ne 0 ]; then
        echo "Échec du téléchargement de Lua."
        exit 1
    fi
fi

# Décompresser Lua si nécessaire
if [ ! -d "$LUA_BUILD_DIR/$LUA_DIR" ]; then
    echo "Décompression de Lua $LUA_VERSION..."
    tar -xzf "$DOWNLOAD_DIR/$LUA_TAR" -C "$LUA_BUILD_DIR"
    if [ $? -ne 0 ]; then
        echo "Échec de la décompression de Lua."
        exit 1
    fi
fi

# Compiler Lua si les fichiers nécessaires sont absents
if [ ! -f "$LUA_LIB" ] || [ ! -f "${LUA_INCLUDE}/lua.h" ]; then
    echo "Compilation de Lua $LUA_VERSION..."
    cd "$LUA_BUILD_DIR/$LUA_DIR" || exit 1
    sed -i 's/^MYCFLAGS=.*/MYCFLAGS=-fPIC/' src/Makefile
    sed -i 's/^MYLIBS=.*/MYLIBS=-lm/' src/Makefile
    sed -i 's/^ALL_T=.*/ALL_T=liblua.a/' src/Makefile

    make clean
    make linux -j"$(nproc)"
    if [ $? -ne 0 ]; then
        echo "Échec de la compilation de Lua."
        exit 1
    fi
else
    echo "Lua $LUA_VERSION est déjà compilé."
fi

# Revenir au répertoire du script
cd "$SCRIPT_DIR" || exit 1


# Télécharger libzip si nécessaire
if [ ! -f "$DOWNLOAD_DIR/$LIBZIP_TAR" ]; then
    echo "Téléchargement de libzip $LIBZIP_VERSION..."
    wget "${LIBZIP_URL}" -O "$DOWNLOAD_DIR/$LIBZIP_TAR"
    if [ $? -ne 0 ]; then
        echo "Échec du téléchargement de libzip."
        exit 1
    fi
fi

# Décompresser libzip si nécessaire
if [ ! -d "$LIBZIP_BUILD_DIR/$LIBZIP_DIR" ]; then
    echo "Décompression de libzip $LIBZIP_VERSION..."
    mkdir -p "$LIBZIP_BUILD_DIR"
    tar -xzf "$DOWNLOAD_DIR/$LIBZIP_TAR" -C "$LIBZIP_BUILD_DIR"
    if [ $? -ne 0 ]; then
        echo "Échec de la décompression de libzip."
        exit 1
    fi
fi


# Chercher libzip.a et zip.h
LIBZIP_LIB_PATH=$(find . -name "$LIBZIP_LIB_NAME" -print -quit | sed 's|^\./||')
LIBZIP_HEADER_PATH=$(find . -name "$LIBZIP_HEADER_NAME" -print -quit | sed 's|^\./||')

# Vérifier si la bibliothèque et les en-têtes sont déjà présents
if [ ! -f "$LIBZIP_LIB_PATH" ] || [ ! -f "$LIBZIP_HEADER_PATH" ]; then
    echo "Compilation de libzip $LIBZIP_VERSION..."
    cd "$LIBZIP_BUILD_DIR/$LIBZIP_DIR" || exit 1
    mkdir -p build
    cd build || exit 1

    # Emplacement par défaut de libz.a
    default_path="/usr/lib/x86_64-linux-gnu/libz.a"

    # Vérifier si libz.a est présent à l'emplacement par défaut
    if [ ! -f "$default_path" ]; then
        echo "libz.a non trouvé à l'emplacement par défaut. Recherche en cours..."

        # Rechercher libz.a sur le système
        libz_path=$(find /usr /usr/local -name "libz.a" 2>/dev/null | head -n 1)
        if [ -z "$libz_path" ]; then
            echo "libz.a non trouvé sur le système."
            exit 1
        fi
        cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DZLIB_LIBRARY="$libz_path" -DZLIB_INCLUDE_DIR=/usr/include -DENABLE_ZSTD=OFF -DENABLE_LZMA=OFF ..
    else
        cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DZLIB_LIBRARY="$default_path" -DZLIB_INCLUDE_DIR=/usr/include -DENABLE_ZSTD=OFF -DENABLE_LZMA=OFF ..
    fi

    make -j"$(nproc)"
    if [ $? -ne 0 ]; then
        echo "Échec de la compilation de libzip."
        exit 1
    fi
else
    echo "libzip $LIBZIP_VERSION est déjà compilé."
fi

# Mettre à jour les chemins de physfs après la compilation
LIBZIP_LIB_PATH="${LIBZIP_BUILD_DIR}/${LIBZIP_DIR}/build/lib/${LIBZIP_LIB_NAME}"

# Revenir au répertoire du script
cd "$SCRIPT_DIR" || exit 1

# Télécharger PhysFS si nécessaire
if [ ! -f "$DOWNLOAD_DIR/$PHYSFS_TAR" ]; then
    echo "Téléchargement de PhysFS $PHYSFS_VERSION..."
    wget "$PHYSFS_URL" -O "$DOWNLOAD_DIR/$PHYSFS_TAR"
    if [ $? -ne 0 ]; then
        echo "Échec du téléchargement de PhysFS."
        exit 1
    fi
fi

# Extraire PhysFS si nécessaire
if [ ! -d "$PHYSFS_BUILD_DIR/$PHYSFS_DIR" ]; then
    echo "Décompression de physfs $PHYSFS_VERSION..."
    mkdir -p "$PHYSFS_BUILD_DIR"
    tar -xvjf "$DOWNLOAD_DIR/$PHYSFS_TAR" -C "$PHYSFS_BUILD_DIR"
    if [ $? -ne 0 ]; then
        echo "Échec de la décompression de physfs."
        exit 1
    fi
fi

# Chercher libphysfs.a et physfs.h
PHYSFS_LIB_PATH=$(find . -name "$PHYSFS_LIB_NAME" -print -quit | sed 's|^\./||')
PHYSFS_HEADER_PATH=$(find . -name "$PHYSFS_HEADER_NAME" -print -quit | sed 's|^\./||')

if [ ! -f "$PHYSFS_LIB_PATH" ] || [ ! -f "$PHYSFS_HEADER_PATH" ]; then
    echo "Compilation de physfs $PHYSFS_VERSION..."
    cd "$PHYSFS_BUILD_DIR/$PHYSFS_DIR" || exit 1
    mkdir -p build
    cd build || exit 1
    cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON ..
    make -j"$(nproc)"
    if [ $? -ne 0 ]; then
        echo "Échec de la compilation de physfs."
        exit 1
    fi
else
    echo "physfs $PHYSFS_VERSION est déjà compilé."
fi

# Mettre à jour les chemins de physfs après la compilation
PHYSFS_LIB_PATH="${PHYSFS_BUILD_DIR}/${PHYSFS_DIR}/build/${PHYSFS_LIB_NAME}"

# Revenir au répertoire du script
cd "$SCRIPT_DIR" || exit 1


# Chercher libbz2.a
BZIP2_LIB_NAME="libbz2.a"
libbz2_path=$(find /usr /usr/local -name "${BZIP2_LIB_NAME}" 2>/dev/null | head -n 1)
libbz2_path_local=$(find . -name "libbz2.a" -print -quit | sed 's|^\./||')
if [ -z "$libbz2_path" ] && [ -z "$libbz2_path_local" ]; then
    echo "bzip2 n'est pas installé et non compilé en local"

    # Télécharger bzip2 si nécessaire
    if [ ! -f "$DOWNLOAD_DIR/$BZIP2_TAR" ]; then
        echo "Téléchargement de bzip2 $BZIP2_VERSION..."
        wget "https://freefr.dl.sourceforge.net/project/bzip2/${BZIP2_TAR}" -O "${DOWNLOAD_DIR}/${BZIP2_TAR}"
        if [ $? -ne 0 ]; then
            echo "Échec du téléchargement de bzip2."
            exit 1
        fi
    fi

    # Décompresser bzip2 si nécessaire
    if [ ! -d "${BZIP2_BUILD_DIR}/${BZIP2_DIR}" ]; then
        echo "Décompression de bzip2 ${BZIP2_VERSION}"
        mkdir -p "${BZIP2_BUILD_DIR}"
        tar -xzf "${DOWNLOAD_DIR}/$BZIP2_TAR" -C "${BZIP2_BUILD_DIR}"
        if [ $? -ne 0 ]; then
            echo "Échec de la décompression de bzip2."
            exit 1
        fi
    fi

    # Chercher libbz2.a
    libbz2_path=$(find . -name "libbz2.a" -print -quit | sed 's|^\./||')
    # Compiler bzip2 si les fichiers nécessaires sont absents
    if [ ! -f "${libbz2_path}" ]; then
        echo "Compilation de bzip2 ${BZIP2_VERSION}..."
        cd "${BZIP2_BUILD_DIR}/${BZIP2_DIR}" || exit 1
        make clean
        make -j"$(nproc)"
    fi
fi

if [ -z $libbz2_path ]; then
    libbz2_path="${BZIP2_BUILD_DIR}/${BZIP2_DIR}/${BZIP2_LIB_NAME}"
fi

# Chercher libssl.a
OPENSSL_LIB_NAME="libssl.a"
OPENSSL_PATH=$(find /usr /usr/local -name "${OPENSSL_LIB_NAME}" 2>/dev/null | head -n 1)
OPENSSL_PATH_LOCAL=$(find . -name "libssl.a" -print -quit | sed 's|^\./||')
CRYPTO_LIB_NAME="libcrypto.a"
CRYPTO_PATH=$(find /usr /usr/local -name "${CRYPTO_LIB_NAME}" 2>/dev/null | head -n 1)
if [ -z "${OPENSSL_PATH}" ] && [ -z "${OPENSSL_PATH_LOCAL}" ]; then
    echo "openssl n'est pas installé et non compilé en local"

    # Télécharger openssl si nécessaire
    if [ ! -f "${DOWNLOAD_DIR}/${OPENSSL_TAR}" ]; then
        echo "Téléchargement de openssl ${OPENSSL_VERSION}..."
        wget "https://www.openssl.org/source/${OPENSSL_TAR}" -O "${DOWNLOAD_DIR}/${OPENSSL_TAR}"
        if [ $? -ne 0 ]; then
            echo "Échec du téléchargement de openssl."
            exit 1
        fi
    fi

    # Décompresser openssl si nécessaire
    if [ ! -d "${OPENSSL_BUILD_DIR}/${OPENSSL_DIR}" ]; then
        echo "Décompression de openssl ${OPENSSL_VERSION}..."
        mkdir -p "${OPENSSL_BUILD_DIR}"
        tar -xzf "${DOWNLOAD_DIR}/${OPENSSL_TAR}" -C "${OPENSSL_BUILD_DIR}"
        if [ $? -ne 0 ]; then
            echo "Échec de la décompression de openssl."
            exit 1
        fi
    fi

    OPENSSL_PATH_LOCAL=$(find . -name "libssl.a" -print -quit | sed 's|^\./||')
    if [ ! -f "${OPENSSL_PATH_LOCAL}" ]; then
        echo "Compilation de openssl ${OPENSSL_VERSION}..."
        cd "${OPENSSL_BUILD_DIR}/${OPENSSL_DIR}" || exit 1
        ./Configure no-shared
        make clean
        make -j"$(nproc)"
    fi
fi

if [ -z ${OPENSSL_PATH} ]; then
    OPENSSL_PATH="${OPENSSL_BUILD_DIR}/${OPENSSL_DIR}/${OPENSSL_LIB_NAME}"
    CRYPTO_PATH="${OPENSSL_BUILD_DIR}/${OPENSSL_DIR}/${CRYPTO_LIB_NAME}"
fi

# Chercher libz.a
LIBZ_LIB_NAME="libz.a"
LIBZ_PATH=$(find /usr /usr/local -name "${LIBZ_LIB_NAME}" 2>/dev/null | head -n 1)
if [ -z "${LIBZ_PATH}" ]; then
    echo "bzip n'est pas installé ni compilé en local"
fi


# Créer le répertoire de build pour le projet
mkdir -p "$PROJECT_BUILD_DIR"
cd "$PROJECT_BUILD_DIR" || exit 1

#echo "lua_lib: ${LUA_LIB}"
#echo "lua_include: ${LUA_INCLUDE}"
#echo "libzip_lib: ${LIBZIP_LIB_PATH}"
#echo "libzip_include: ${LIBZIP_INCLUDE}"
#echo "physfs_lib: ${PHYSFS_LIB_PATH}"
#echo "physfs_include: ${PHYSFS_INCLUDE}"
#
#echo "libbz2_path: ${libbz2_path}"
#echo "openssl_path: ${OPENSSL_PATH}"
#echo "crypto_path: ${CRYPTO_PATH}"
#echo "libz_path: ${LIBZ_PATH}"


cmake "$SCRIPT_DIR" -DLUA_LIB="$LUA_LIB" \
      -DLUA_INCLUDE="$LUA_INCLUDE" \
      -DLIBZIP_LIB="$LIBZIP_LIB_PATH" \
      -DLIBZIP_INCLUDE="$LIBZIP_INCLUDE" \
      -DPHYSFS_LIB="$PHYSFS_LIB_PATH" \
      -DPHYSFS_INCLUDE="$PHYSFS_INCLUDE" \
      -DBZIP2_LIB="${libbz2_path}" \
      -DOPENSSL_LIB="${OPENSSL_PATH}" \
      -DCRYPTO_LIB="${CRYPTO_PATH}" \
      -DLIBZ_LIB="${LIBZ_PATH}"
if [ $? -ne 0 ]; then
    echo "Échec de la configuration avec CMake."
    exit 1
fi

# Compiler le projet
make -j"$(nproc)"
if [ $? -ne 0 ]; then
    echo "Échec de la compilation du projet."
    exit 1
fi

if [ -f "${SCRIPT_DIR}/test/${PROJECT_NAME}" ]; then
  rm "${SCRIPT_DIR}/test/${PROJECT_NAME}"
fi

cp "${PROJECT_BUILD_DIR}/${PROJECT_NAME}" "${SCRIPT_DIR}/test/${PROJECT_NAME}"

cd "${SCRIPT_DIR}/test/"

(./"${PROJECT_NAME}" .)



# # Créer le répertoire de construction s'il n'existe pas
# if [ ! -d "$BUILD_DIR" ]; then
#     mkdir "$BUILD_DIR"
# fi

# # Aller dans le répertoire de construction
# cd "$BUILD_DIR" || exit

# # Exécuter cmake et vérifier les erreurs
# if cmake -G Ninja -DLUA_INCLUDE_DIR="$EXTERNAL_DIR/include" -DLUA_LIBRARY="$EXTERNAL_DIR/lib/liblua.a" -DOPENSSL_ROOT_DIR="$OPENSSL_ROOT_DIR" ..; then
#     echo "Configuration CMake réussie."
# else
#     echo "Échec de la configuration CMake."
#     bash "$CLEAR_SCRIPT"
#     exit 1
# fi

# # Compiler le code et vérifier les erreurs
# if ninja; then
#     echo "Compilation réussie."
# else
#     echo "Échec de la compilation."
#     bash "$CLEAR_SCRIPT"
#     exit 1
# fi

# # Rendre le binaire exécutable
# chmod +x "$BINAIRE_NOM"

# echo "Processus de construction et de déploiement terminé avec succès."
