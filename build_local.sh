#!/bin/bash

# Arrêt au premier échec : si un wget/cmake/make foire silencieusement,
# on ne veut pas continuer à compiler avec un état corrompu. Les commandes
# critiques sont déjà testées explicitement avec `if !`, mais set -e fait
# office de filet de sécurité pour celles qu'on aurait oubliées.
set -e

# Parsing des arguments
RUN_AFTER_BUILD=0

for arg in "$@"; do
    case "$arg" in
        --run)
            RUN_AFTER_BUILD=1
            ;;
        --help|-h)
            echo "Usage: $0 [--run]"
            echo "  (par défaut)   Compile uniquement"
            echo "  --run          Compile puis exécute le binaire sur test/"
            exit 0
            ;;
        *)
            echo "Argument inconnu : $arg"
            echo "Voir $0 --help"
            exit 1
            ;;
    esac
done

# Vérifier si CMake est installé
if ! command -v cmake &> /dev/null; then
    echo "CMake n'est pas installé. Veuillez l'installer avant de continuer."
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
    exit 1
fi

# Vérifier que unzip est installé
if ! command -v unzip &> /dev/null; then
    echo "unzip n'est pas installé. Veuillez l'installer avant de continuer."
    exit 1
fi

# --- Vérification d'intégrité des dépendances téléchargées ---------
# Chaîne d'approvisionnement : tout artefact distant est vérifié contre
# un SHA256 épinglé AVANT d'être utilisé. Comportement strict :
#   - hash non renseigné  -> build refusé (exit 1). Un checksum
#     optionnel n'est pas un checksum : le jour d'un bump de version
#     où on oublie de recalculer, on veut un échec, pas un trou muet.
#   - hash ne correspond pas -> fichier suspect supprimé + exit 1.
#     Pas de fallback, pas de "on continue quand même".
# La vérification s'applique AUSSI aux fichiers déjà en cache dans
# downloads/ : un cache empoisonné doit être détecté, sinon le
# mécanisme ne sert à rien.
verify_sha256() {
    local file="$1"
    local expected="$2"
    local name="$3"

    if [ -z "${expected}" ]; then
        echo "ERREUR: checksum SHA256 non renseigné pour ${name}."
        echo "  Build refusé tant que la variable ${name}_SHA256 est vide."
        echo "  Calcule le hash depuis la source OFFICIELLE (voir le"
        echo "  commentaire au-dessus de la variable) puis renseigne-le."
        exit 1
    fi

    if [ ! -f "${file}" ]; then
        echo "ERREUR: fichier introuvable pour vérification (${name}): ${file}"
        exit 1
    fi

    local actual=""
    if command -v sha256sum &> /dev/null; then
        actual="$(sha256sum "${file}" | awk '{print $1}')"
    elif command -v shasum &> /dev/null; then
        actual="$(shasum -a 256 "${file}" | awk '{print $1}')"
    else
        echo "ERREUR: ni sha256sum ni shasum -a 256 disponibles."
        echo "  Impossible de vérifier l'intégrité de ${name}."
        exit 1
    fi

    if [ "${actual}" != "${expected}" ]; then
        echo "ERREUR: checksum SHA256 invalide pour ${name}."
        echo "  attendu : ${expected}"
        echo "  obtenu  : ${actual}"
        echo "  Fichier supprimé (suspect) : ${file}"
        rm -f "${file}"
        exit 1
    fi

    echo "Checksum SHA256 OK pour ${name}."
}

# Variables
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
DOWNLOAD_DIR="${SCRIPT_DIR}/downloads"
#
PROJECT_BUILD_DIR="${BUILD_DIR}/project_build"
PROJECT_NAME="luapilot"
#
# LUA_VERSION="5.4.7"
LUA_VERSION="5.5.0"
LUA_DIR="lua-$LUA_VERSION"
LUA_TAR="$LUA_DIR.tar.gz"
LUA_BUILD_DIR="${BUILD_DIR}/lua_build"
LUA_URL="https://www.lua.org/ftp/$LUA_TAR"
# Pour renseigner LUA_SHA256, calcule le hash depuis la source
# officielle (laisse vide = build refusé) :
#   wget -qO- https://www.lua.org/ftp/lua-5.5.0.tar.gz | sha256sum
LUA_SHA256="57ccc32bbbd005cab75bcc52444052535af691789dba2b9016d5c50640d68b3d"
LUA_LIB_NAME="liblua.a"
LUA_LIB="${LUA_BUILD_DIR}/${LUA_DIR}/src/${LUA_LIB_NAME}"
LUA_INCLUDE="${LUA_BUILD_DIR}/${LUA_DIR}/src"
#
# OPENSSL_VERSION="3.3.1"
OPENSSL_VERSION="3.5.6"
OPENSSL_DIR="openssl-${OPENSSL_VERSION}"
OPENSSL_TAR="${OPENSSL_DIR}.tar.gz"
OPENSSL_BUILD_DIR="${BUILD_DIR}/openssl"
# Dépendance crypto : la plus critique à vérifier. Calcule :
#   wget -qO- https://www.openssl.org/source/openssl-3.5.6.tar.gz | sha256sum
# et recoupe avec le hash publié officiellement par le projet :
#   wget -qO- https://www.openssl.org/source/openssl-3.5.6.tar.gz.sha256
OPENSSL_SHA256="deae7c80cba99c4b4f940ecadb3c3338b13cb77418409238e57d7f31f2a3b736"
#
MINIZ_VERSION="3.1.1"
MINIZ_DIR="miniz-${MINIZ_VERSION}"
MINIZ_ZIP="${MINIZ_DIR}.zip"
MINIZ_URL="https://github.com/richgel999/miniz/releases/download/${MINIZ_VERSION}/${MINIZ_ZIP}"
# Calcule :
#   wget -qO- https://github.com/richgel999/miniz/releases/download/3.1.1/miniz-3.1.1.zip | sha256sum
MINIZ_SHA256="cb28402bb2af93bdc331b60d16807e89727d1712a2d0a7ba0cac79a3e406fe40"
MINIZ_BUILD_DIR="${BUILD_DIR}/miniz"
MINIZ_INSTALL_DIR="${MINIZ_BUILD_DIR}/${MINIZ_DIR}"
MINIZ_C="${MINIZ_INSTALL_DIR}/miniz.c"
MINIZ_H="${MINIZ_INSTALL_DIR}/miniz.h"
#
# nlohmann/json : header-only, un seul fichier json.hpp téléchargé
# depuis les releases GitHub. Version épinglée comme pour miniz ; pour
# en changer, il suffit de bumper JSON_VERSION.
JSON_VERSION="3.11.3"
JSON_DIR="json-${JSON_VERSION}"
JSON_HEADER="json.hpp"
JSON_URL="https://github.com/nlohmann/json/releases/download/v${JSON_VERSION}/json.hpp"
# Calcule :
#   wget -qO- https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp | sha256sum
JSON_SHA256="9bea4c8066ef4a1c206b2be5a36302f8926f7fdc6087af5d20b417d0cf103ea6"
JSON_BUILD_DIR="${BUILD_DIR}/json"
JSON_INSTALL_DIR="${JSON_BUILD_DIR}/${JSON_DIR}"
# On place le header sous un sous-dossier nlohmann/ pour que
# #include <nlohmann/json.hpp> fonctionne avec JSON_INSTALL_DIR seul
# ajouté aux include paths.
JSON_INCLUDE_FILE="${JSON_INSTALL_DIR}/nlohmann/json.hpp"
#
GENERATED_DIR="${BUILD_DIR}/generated"


# Créer les répertoires nécessaires
mkdir -p "$DOWNLOAD_DIR"
mkdir -p "$LUA_BUILD_DIR"
mkdir -p "$BUILD_DIR"


# Installer miniz si nécessaire
if [ ! -f "${MINIZ_C}" ] || [ ! -f "${MINIZ_H}" ]; then
    echo "Installation de miniz ${MINIZ_VERSION}..."
    mkdir -p "${MINIZ_INSTALL_DIR}"

    if [ ! -f "${DOWNLOAD_DIR}/${MINIZ_ZIP}" ]; then
        echo "Téléchargement de miniz ${MINIZ_VERSION}..."
        if ! wget "${MINIZ_URL}" -O "${DOWNLOAD_DIR}/${MINIZ_ZIP}"; then
            echo "Échec du téléchargement de miniz."
            rm -f "${DOWNLOAD_DIR}/${MINIZ_ZIP}"
            exit 1
        fi
    fi

    # Vérifié même si déjà en cache (cache empoisonné).
    verify_sha256 "${DOWNLOAD_DIR}/${MINIZ_ZIP}" "${MINIZ_SHA256}" "miniz"

    TEMP_MINIZ="${BUILD_DIR}/miniz_extract"
    rm -rf "${TEMP_MINIZ}"
    mkdir -p "${TEMP_MINIZ}"
    unzip -q "${DOWNLOAD_DIR}/${MINIZ_ZIP}" -d "${TEMP_MINIZ}"
    if [ $? -ne 0 ]; then
        echo "Échec de la décompression de miniz."
        exit 1
    fi

    # La structure interne de l'archive miniz peut varier selon la version,
    # on cherche donc miniz.c et miniz.h où qu'ils soient.
    FOUND_C=$(find "${TEMP_MINIZ}" -name "miniz.c" -print -quit)
    FOUND_H=$(find "${TEMP_MINIZ}" -name "miniz.h" -print -quit)

    if [ -z "${FOUND_C}" ] || [ -z "${FOUND_H}" ]; then
        echo "Erreur : miniz.c ou miniz.h introuvables dans l'archive téléchargée."
        exit 1
    fi

    cp "${FOUND_C}" "${MINIZ_C}"
    cp "${FOUND_H}" "${MINIZ_H}"
    rm -rf "${TEMP_MINIZ}"
    echo "miniz ${MINIZ_VERSION} installé."
fi


# Installer nlohmann/json (header-only) si nécessaire
if [ ! -f "${JSON_INCLUDE_FILE}" ]; then
    echo "Installation de nlohmann/json ${JSON_VERSION}..."
    mkdir -p "${JSON_INSTALL_DIR}/nlohmann"

    if [ ! -f "${DOWNLOAD_DIR}/${JSON_DIR}-${JSON_HEADER}" ]; then
        echo "Téléchargement de nlohmann/json ${JSON_VERSION}..."
        if ! wget "${JSON_URL}" -O "${DOWNLOAD_DIR}/${JSON_DIR}-${JSON_HEADER}"; then
            echo "Échec du téléchargement de nlohmann/json."
            rm -f "${DOWNLOAD_DIR}/${JSON_DIR}-${JSON_HEADER}"
            exit 1
        fi
    fi

    # Vérifié même si déjà en cache (cache empoisonné).
    verify_sha256 "${DOWNLOAD_DIR}/${JSON_DIR}-${JSON_HEADER}" "${JSON_SHA256}" "nlohmann/json"

    cp "${DOWNLOAD_DIR}/${JSON_DIR}-${JSON_HEADER}" "${JSON_INCLUDE_FILE}"
    echo "nlohmann/json ${JSON_VERSION} installé."
fi


# Télécharger Lua si nécessaire
if [ ! -f "$DOWNLOAD_DIR/$LUA_TAR" ]; then
    echo "Téléchargement de Lua $LUA_VERSION..."
    if ! wget "${LUA_URL}" -O "$DOWNLOAD_DIR/$LUA_TAR"; then
        echo "Échec du téléchargement de Lua."
        rm -f "$DOWNLOAD_DIR/$LUA_TAR"
        exit 1
    fi
fi

# Vérifié même si déjà en cache (cache empoisonné).
verify_sha256 "$DOWNLOAD_DIR/$LUA_TAR" "${LUA_SHA256}" "Lua"

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


# Chercher libssl.a
OPENSSL_LIB_NAME="libssl.a"
OPENSSL_PATH=$(find /usr /usr/local -name "${OPENSSL_LIB_NAME}" 2>/dev/null | head -n 1)
OPENSSL_PATH_LOCAL=$(find . -name "libssl.a" -print -quit | sed 's|^\./||')
CRYPTO_LIB_NAME="libcrypto.a"
CRYPTO_PATH=$(find /usr /usr/local -name "${CRYPTO_LIB_NAME}" 2>/dev/null | head -n 1)
if [ -z "${OPENSSL_PATH}" ] && [ -z "${OPENSSL_PATH_LOCAL}" ]; then
    echo "openssl n'est pas installé et non compilé en local"

    if [ ! -f "${DOWNLOAD_DIR}/${OPENSSL_TAR}" ]; then
        echo "Téléchargement de openssl ${OPENSSL_VERSION}..."
        if ! wget "https://www.openssl.org/source/${OPENSSL_TAR}" -O "${DOWNLOAD_DIR}/${OPENSSL_TAR}"; then
            echo "Échec du téléchargement de openssl."
            rm -f "${DOWNLOAD_DIR}/${OPENSSL_TAR}"
            exit 1
        fi
    fi

    # Dépendance crypto : vérifiée avant toute compilation/lien, même
    # si déjà en cache.
    verify_sha256 "${DOWNLOAD_DIR}/${OPENSSL_TAR}" "${OPENSSL_SHA256}" "openssl"

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

if [ -z "${OPENSSL_PATH}" ]; then
    OPENSSL_PATH="${OPENSSL_BUILD_DIR}/${OPENSSL_DIR}/${OPENSSL_LIB_NAME}"
    CRYPTO_PATH="${OPENSSL_BUILD_DIR}/${OPENSSL_DIR}/${CRYPTO_LIB_NAME}"
fi

# Revenir au répertoire du script
cd "$SCRIPT_DIR" || exit 1


# Créer le répertoire de build pour le projet
mkdir -p "$PROJECT_BUILD_DIR"
cd "$PROJECT_BUILD_DIR" || exit 1

# --- Génération des modules Lua bundlés ---------------------------
# On régénère systématiquement : c'est rapide et ça garantit qu'un
# vendor/*.lua modifié sera repris au prochain build sans clear.sh.
echo "Génération des modules Lua bundlés..."
mkdir -p "${GENERATED_DIR}"
bash "${SCRIPT_DIR}/tools/embed_lua_module.sh" \
    "${SCRIPT_DIR}/vendor/inspect.lua" \
    "${GENERATED_DIR}/embedded_inspect.hpp" \
    "inspect"

cmake "$SCRIPT_DIR" \
    -DLUA_LIB="$LUA_LIB" \
    -DLUA_INCLUDE="$LUA_INCLUDE" \
    -DOPENSSL_LIB="${OPENSSL_PATH}" \
    -DCRYPTO_LIB="${CRYPTO_PATH}" \
    -DMINIZ_SRC="${MINIZ_C}" \
    -DMINIZ_INCLUDE="${MINIZ_INSTALL_DIR}" \
    -DJSON_INCLUDE="${JSON_INSTALL_DIR}" \
    -DGENERATED_INCLUDE="${GENERATED_DIR}"
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


# Reset complet du dossier test pour partir d'un état propre
rm -rf "${SCRIPT_DIR}/test"
mkdir -p "${SCRIPT_DIR}/test"

# Copier le binaire compilé
cp "${PROJECT_BUILD_DIR}/${PROJECT_NAME}" "${SCRIPT_DIR}/test/${PROJECT_NAME}"

# Copier le contenu de examples/ vers test/ (s'il existe)
if [ -d "${SCRIPT_DIR}/examples" ]; then
    cp -r "${SCRIPT_DIR}/examples/." "${SCRIPT_DIR}/test/"
fi

echo "Build OK. Binaire prêt dans ${SCRIPT_DIR}/test/${PROJECT_NAME}"

# Si --run : lance le binaire sur le dossier test
if [ "${RUN_AFTER_BUILD}" -eq 1 ]; then
    echo "Lancement du binaire sur test/..."
    cd "${SCRIPT_DIR}/test/"
    (./"${PROJECT_NAME}" .)
fi
