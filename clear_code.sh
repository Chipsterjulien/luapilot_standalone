#!/bin/bash
# Usage:
#   ./clear.sh           # nettoie build/ et test/ (rebuild rapide)
#   ./clear.sh --all     # nettoie aussi downloads/ (vraie reset)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

REMOVE_DOWNLOADS=0

for arg in "$@"; do
    case "$arg" in
        --all)
            REMOVE_DOWNLOADS=1
            ;;
        --help|-h)
            echo "Usage: $0 [--all]"
            echo "  (par défaut)   Nettoie build/ et test/"
            echo "  --all          Nettoie aussi downloads/ (force le re-téléchargement)"
            exit 0
            ;;
        *)
            echo "Argument inconnu : $arg"
            echo "Voir $0 --help"
            exit 1
            ;;
    esac
done

# Artefacts de compilation
if [ -d "${SCRIPT_DIR}/build" ]; then
    echo "Suppression de build/..."
    rm -rf "${SCRIPT_DIR}/build"
fi

# Dossier de test (recréé à chaque build)
if [ -d "${SCRIPT_DIR}/test" ]; then
    echo "Suppression de test/..."
    rm -rf "${SCRIPT_DIR}/test"
fi

# Hérité de l'ancien emplacement de miniz, au cas où il traîne encore
if [ -d "${SCRIPT_DIR}/src/third_party" ]; then
    echo "Suppression de src/third_party/ (legacy)..."
    rm -rf "${SCRIPT_DIR}/src/third_party"
fi

# Archives téléchargées (optionnel)
if [ "$REMOVE_DOWNLOADS" -eq 1 ] && [ -d "${SCRIPT_DIR}/downloads" ]; then
    echo "Suppression de downloads/..."
    rm -rf "${SCRIPT_DIR}/downloads"
fi

echo "Terminé."