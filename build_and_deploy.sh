#!/bin/bash

# Vérifier si le script est exécuté en tant que root
if [ "$(id -u)" -ne 0 ]; then
    echo "Ce script doit être exécuté en tant que root."
    exit 1
fi

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

# Variables
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
DOWNLOAD_DIR="${SCRIPT_DIR}/downloads"
BINAIRE_NOM="luapilot"
INSTALL_PATH="/usr/bin/$BINAIRE_NOM"
CLEAR_SCRIPT="${SCRIPT_DIR}/clear_code.sh"
LUA_VERSION="5.3.6"
LUA_DIR="lua-$LUA_VERSION"
LUA_TAR="$LUA_DIR.tar.gz"
LUA_URL="http://www.lua.org/ftp/$LUA_TAR"
LIBZIP_VERSION="1.8.0"
LIBZIP_DIR="libzip-$LIBZIP_VERSION"
LIBZIP_TAR="$LIBZIP_DIR.tar.gz"
LIBZIP_URL="https://libzip.org/download/$LIBZIP_TAR"
PHYSFS_VERSION="release-3.2.0"
PHYSFS_DIR="physfs-$PHYSFS_VERSION"
PHYSFS_ZIP="release-3.2.0.zip"
PHYSFS_URL="https://github.com/icculus/physfs/archive/refs/tags/$PHYSFS_ZIP"
EXTERNAL_DIR="${SCRIPT_DIR}/external/lua"
LIB_DIR="${SCRIPT_DIR}/libs"
INCLUDE_DIR="${SCRIPT_DIR}/include"
OPENSSL_ROOT_DIR="/usr/include/openssl"

# Créer les répertoires nécessaires
mkdir -p "$DOWNLOAD_DIR"
mkdir -p "$LIB_DIR"
mkdir -p "$INCLUDE_DIR"

# Lancer le script clear_code.sh et vérifier les erreurs
if [ -f "$CLEAR_SCRIPT" ]; then
    if bash "$CLEAR_SCRIPT"; then
        echo "clear_code.sh exécuté avec succès."
    else
        echo "Échec de l'exécution de clear_code.sh."
        exit 1
    fi
else
    echo "Le script $CLEAR_SCRIPT n'existe pas."
    exit 1
fi

# Télécharger et compiler Lua statique
if [ ! -d "$EXTERNAL_DIR" ]; then
    mkdir -p "$EXTERNAL_DIR/lib"
    mkdir -p "$EXTERNAL_DIR/include"
    cd "$DOWNLOAD_DIR" || exit

    if [ ! -f "$LUA_TAR" ]; then
        wget "$LUA_URL"
    fi

    tar -xvf "$LUA_TAR"
    cd "$LUA_DIR" || exit

    sed -i 's/^MYCFLAGS=.*/MYCFLAGS=-fPIC/' src/Makefile
    sed -i 's/^MYLIBS=.*/MYLIBS=-lm/' src/Makefile
    sed -i 's/^ALL_T=.*/ALL_T=liblua.a/' src/Makefile

    make linux
    cp src/liblua.a "$EXTERNAL_DIR/lib/"
    cp src/*.h "$EXTERNAL_DIR/include/"
    cp src/*.hpp "$EXTERNAL_DIR/include/"
    cd "$SCRIPT_DIR"
fi

# Télécharger et compiler libzip en statique
if [ ! -f "$LIB_DIR/libzip.a" ]; then
    cd "$DOWNLOAD_DIR" || exit
    if [ ! -f "$LIBZIP_TAR" ]; then
        wget "$LIBZIP_URL"
    fi
    tar -xzvf "$LIBZIP_TAR"
    cd "$LIBZIP_DIR" || exit
    mkdir build
    cd build || exit
    cmake -DBUILD_SHARED_LIBS=OFF ..
    make
    cp lib/libzip.a "$LIB_DIR/"
    cp ../lib/zip.h "$INCLUDE_DIR/"
    cd "$SCRIPT_DIR"
    rm -rf "$DOWNLOAD_DIR/$LIBZIP_DIR"
fi

# Télécharger et compiler physfs en statique
if [ ! -f "$LIB_DIR/libphysfs.a" ]; then
    cd "$DOWNLOAD_DIR" || exit
    if [ ! -f "$PHYSFS_TAR" ]; then
        wget "$PHYSFS_URL"
    fi
    tar -xvjf "$PHYSFS_TAR"
    cd "$PHYSFS_DIR" || exit
    mkdir build
    cd build || exit
    cmake -DBUILD_SHARED_LIBS=OFF ..
    make
    cp libphysfs.a "$LIB_DIR/"
    cp ../physfs.h "$INCLUDE_DIR/"
    cd "$SCRIPT_DIR"
    rm -rf "$DOWNLOAD_DIR/$PHYSFS_DIR"
fi

# Créer le répertoire de construction s'il n'existe pas
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

# Aller dans le répertoire de construction
cd "$BUILD_DIR" || exit

# Exécuter cmake et vérifier les erreurs
if cmake -G Ninja -DLUA_INCLUDE_DIR="$EXTERNAL_DIR/include" -DLUA_LIBRARY="$EXTERNAL_DIR/lib/liblua.a" -DOPENSSL_ROOT_DIR="$OPENSSL_ROOT_DIR" ..; then
    echo "Configuration CMake réussie."
else
    echo "Échec de la configuration CMake."
    bash "$CLEAR_SCRIPT"
    exit 1
fi

# Compiler le code et vérifier les erreurs
if ninja; then
    echo "Compilation réussie."
else
    echo "Échec de la compilation."
    bash "$CLEAR_SCRIPT"
    exit 1
fi

# Compression avec UPX
if command -v upx &> /dev/null; then
    upx --best "$BINAIRE_NOM"
else
    echo "UPX n'est pas installé. Veuillez l'installer pour compresser le binaire."
fi

# Compression avec UPX
if command -v upx &> /dev/null; then
    upx --best "$BINAIRE_NOM"
else
    echo "UPX n'est pas installé. Veuillez l'installer pour compresser le binaire."
fi

# Rendre le binaire exécutable
chmod +x "$BINAIRE_NOM"

# Copier le binaire dans /usr/bin et vérifier les erreurs
if cp "$BINAIRE_NOM" "$INSTALL_PATH"; then
    echo "Binaire copié avec succès dans $INSTALL_PATH."
else
    echo "Échec de la copie du binaire."
    bash "$CLEAR_SCRIPT"
    exit 1
fi

# Lancer le script clear_code.sh et vérifier les erreurs
if [ -f "$CLEAR_SCRIPT" ]; then
    if bash "$CLEAR_SCRIPT"; then
        echo "clear_code.sh exécuté avec succès."
    else
        echo "Échec de l'exécution de clear_code.sh."
        exit 1
    fi
else
    echo "Le script $CLEAR_SCRIPT n'existe pas."
    exit 1
fi

echo "Processus de construction et de déploiement terminé avec succès."
