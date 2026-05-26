#!/bin/bash
# =====================================================================
# release.sh — automatise la fabrication d'une release LuaPilot
# =====================================================================
#
# Ce que le script fait :
#   1. (optionnel) build via build_local.sh  --> --build
#   2. copie du binaire dans dist/
#   3. strip du binaire
#   4. sha256 du binaire strippé
#   5. tarball luapilot-<version>-linux-<arch>.tar.gz
#      (contient : luapilot, README.md, README_fr.md, LICENSE, notes.md)
#   6. sha256 du tarball
#
# Sortie : tout dans le répertoire dist/ à la racine du projet.
# Ce dossier est ignoré par git (cf. .gitignore).
#
# Usage :
#   ./release.sh                  # utilise le binaire existant
#   ./release.sh --build          # rebuild d'abord (recommandé pour release)
#   ./release.sh --version 1.2.0  # définit explicitement la version
#   ./release.sh -h               # aide
#
# Si --version n'est pas fourni, le script utilise le dernier tag git
# (ex. "v1.2.0" -> "1.2.0"). En l'absence de tag, version = "unversioned".
# =====================================================================

set -euo pipefail

# --- Parsing des arguments -------------------------------------------
DO_BUILD=0
VERSION=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build)
            DO_BUILD=1
            shift
            ;;
        --version)
            if [[ $# -lt 2 ]]; then
                echo "ERREUR: --version exige un argument" >&2
                exit 1
            fi
            VERSION="$2"
            shift 2
            ;;
        -h|--help)
            sed -n '2,30p' "$0"
            exit 0
            ;;
        *)
            echo "ERREUR: argument inconnu '$1'" >&2
            echo "Voir $0 --help" >&2
            exit 1
            ;;
    esac
done

# --- Localisation projet ---------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

PROJECT_NAME="luapilot"
BUILT_BINARY="${SCRIPT_DIR}/build/project_build/${PROJECT_NAME}"
DIST_DIR="${SCRIPT_DIR}/dist"

# --- Détection de l'architecture -------------------------------------
ARCH="$(uname -m)"
case "${ARCH}" in
    x86_64|amd64) ARCH_TAG="linux-x86_64" ;;
    aarch64|arm64) ARCH_TAG="linux-aarch64" ;;
    *) ARCH_TAG="linux-${ARCH}" ;;
esac

# --- Détection de la version -----------------------------------------
if [[ -z "${VERSION}" ]]; then
    # Tentative : dernier tag git de la forme vX.Y.Z
    if command -v git &>/dev/null && [[ -d "${SCRIPT_DIR}/.git" ]]; then
        GIT_TAG="$(git -C "${SCRIPT_DIR}" describe --tags --abbrev=0 2>/dev/null || true)"
        if [[ -n "${GIT_TAG}" ]]; then
            # Strip leading 'v' si présent : "v1.2.0" -> "1.2.0"
            VERSION="${GIT_TAG#v}"
        fi
    fi
    if [[ -z "${VERSION}" ]]; then
        VERSION="unversioned"
    fi
fi

echo "=========================================="
echo "  LuaPilot release builder"
echo "  Version : ${VERSION}"
echo "  Arch    : ${ARCH_TAG}"
echo "=========================================="
echo ""

# --- Vérification des outils requis ----------------------------------
for tool in sha256sum strip tar; do
    if ! command -v "${tool}" &>/dev/null; then
        echo "ERREUR: '${tool}' introuvable dans PATH" >&2
        exit 1
    fi
done

# --- Étape 1 : build (optionnel) -------------------------------------
if [[ "${DO_BUILD}" -eq 1 ]]; then
    echo "[1/6] Build via build_local.sh..."
    if [[ ! -x "${SCRIPT_DIR}/build_local.sh" ]]; then
        echo "ERREUR: build_local.sh introuvable ou non exécutable" >&2
        exit 1
    fi
    bash "${SCRIPT_DIR}/build_local.sh"
    echo "      Build terminé."
else
    echo "[1/6] Skip build (utilise le binaire existant)."
fi

# --- Vérification du binaire ----------------------------------------
if [[ ! -f "${BUILT_BINARY}" ]]; then
    echo "ERREUR: binaire introuvable à ${BUILT_BINARY}" >&2
    echo "  Lance avec --build pour rebuild, ou exécute build_local.sh d'abord." >&2
    exit 1
fi

# --- Préparation de dist/ -------------------------------------------
echo "[2/6] Préparation de dist/..."
rm -rf "${DIST_DIR}"
mkdir -p "${DIST_DIR}"

# Copie du binaire dans dist/ (on ne touche pas à l'original).
cp "${BUILT_BINARY}" "${DIST_DIR}/${PROJECT_NAME}"
echo "      Binaire copié : dist/${PROJECT_NAME}"

# --- Étape 3 : strip ------------------------------------------------
echo "[3/6] Strip du binaire..."
SIZE_BEFORE=$(stat -c%s "${DIST_DIR}/${PROJECT_NAME}")
strip "${DIST_DIR}/${PROJECT_NAME}"
SIZE_AFTER=$(stat -c%s "${DIST_DIR}/${PROJECT_NAME}")
echo "      Taille : ${SIZE_BEFORE} -> ${SIZE_AFTER} octets"

# --- Étape 4 : sha256 du binaire ------------------------------------
echo "[4/6] Calcul SHA256 du binaire..."
BINARY_NAME_VERSIONED="${PROJECT_NAME}-${VERSION}-${ARCH_TAG}"
# Le fichier sha256 référence un nom versionné pour qu'on puisse le
# distribuer séparément du binaire et que ça reste clair.
BINARY_SHA256_FILE="${DIST_DIR}/${BINARY_NAME_VERSIONED}.sha256"

# On calcule depuis dist/ avec un nom relatif lisible.
(cd "${DIST_DIR}" && sha256sum "${PROJECT_NAME}") \
    | awk -v name="${BINARY_NAME_VERSIONED}" '{print $1"  "name}' \
    > "${BINARY_SHA256_FILE}"
echo "      SHA256 binaire : $(awk '{print $1}' "${BINARY_SHA256_FILE}")"

# --- Étape 5 : tarball ----------------------------------------------
echo "[5/6] Création du tarball..."
TARBALL_BASENAME="${PROJECT_NAME}-${VERSION}-${ARCH_TAG}"
TARBALL_FILE="${DIST_DIR}/${TARBALL_BASENAME}.tar.gz"

# Liste des fichiers à inclure. README_fr.md et notes.md sont
# optionnels (ne plantent pas le script s'ils manquent).
INCLUDE_FILES=("${PROJECT_NAME}")
[[ -f "${SCRIPT_DIR}/README.md" ]]    && INCLUDE_FILES+=("README.md")    || echo "      WARN: README.md absent" >&2
[[ -f "${SCRIPT_DIR}/README_fr.md" ]] && INCLUDE_FILES+=("README_fr.md")
[[ -f "${SCRIPT_DIR}/LICENSE" ]]      && INCLUDE_FILES+=("LICENSE")      || { echo "ERREUR: LICENSE introuvable à la racine" >&2 ; exit 1; }
[[ -f "${SCRIPT_DIR}/notes.md" ]]     && INCLUDE_FILES+=("notes.md")

# Stage : on copie les fichiers à inclure dans un sous-dossier nommé
# explicitement, pour que le tarball s'extraie en luapilot-X.Y.Z-arch/
STAGE_DIR="${DIST_DIR}/${TARBALL_BASENAME}"
mkdir -p "${STAGE_DIR}"
cp "${DIST_DIR}/${PROJECT_NAME}" "${STAGE_DIR}/"
for f in "${INCLUDE_FILES[@]:1}"; do
    cp "${SCRIPT_DIR}/${f}" "${STAGE_DIR}/"
done

# Création du tarball.
(cd "${DIST_DIR}" && tar -czf "${TARBALL_BASENAME}.tar.gz" "${TARBALL_BASENAME}")

# Nettoyage du stage.
rm -rf "${STAGE_DIR}"

echo "      Tarball : $(basename "${TARBALL_FILE}")"
echo "      Contenu :"
tar -tzf "${TARBALL_FILE}" | sed 's/^/        /'

# --- Étape 6 : sha256 du tarball ------------------------------------
echo "[6/6] Calcul SHA256 du tarball..."
TARBALL_SHA256_FILE="${TARBALL_FILE}.sha256"
(cd "${DIST_DIR}" && sha256sum "${TARBALL_BASENAME}.tar.gz") > "${TARBALL_SHA256_FILE}"
echo "      SHA256 tarball : $(awk '{print $1}' "${TARBALL_SHA256_FILE}")"

# --- Résumé ---------------------------------------------------------
echo ""
echo "=========================================="
echo "  RELEASE PRÊTE dans dist/"
echo "=========================================="
ls -lh "${DIST_DIR}/" | awk 'NR>1 {printf "  %s  %s\n", $5, $9}'
echo ""
echo "Tu peux maintenant uploader ces fichiers sur la page Releases GitHub :"
echo "  - $(basename "${TARBALL_FILE}")"
echo "  - $(basename "${TARBALL_SHA256_FILE}")"
echo ""
echo "Le binaire seul et son sha256 sont aussi dans dist/ si tu veux les"
echo "distribuer séparément :"
echo "  - ${PROJECT_NAME}"
echo "  - $(basename "${BINARY_SHA256_FILE}")"
echo ""