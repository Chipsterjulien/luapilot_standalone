#!/bin/bash

# Arrêt au premier échec : si un wget/cmake/make foire silencieusement,
# on ne veut pas continuer à compiler avec un état corrompu. Les commandes
# critiques sont déjà testées explicitement avec `if !`, mais set -e fait
# office de filet de sécurité pour celles qu'on aurait oubliées.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_NAME="babet"
INSTALL_PATH="/usr/local/bin/${PROJECT_NAME}"

# Compile via le script normal (pas besoin de root pour ça)
if ! bash "${SCRIPT_DIR}/build_local.sh"; then
    echo "Échec de la compilation."
    exit 1
fi

BINARY="${SCRIPT_DIR}/test/${PROJECT_NAME}"
if [ ! -f "${BINARY}" ]; then
    echo "Erreur : binaire ${BINARY} introuvable après compilation."
    exit 1
fi

# Compression UPX optionnelle
if command -v upx &> /dev/null; then
    echo "Compression du binaire avec UPX..."
    upx --best "${BINARY}"
else
    echo "UPX absent, on saute la compression."
fi

# Installation (cette partie a besoin de root)
echo "Installation vers ${INSTALL_PATH}..."
if [ "$(id -u)" -ne 0 ]; then
    sudo cp "${BINARY}" "${INSTALL_PATH}" || exit 1
    sudo chmod +x "${INSTALL_PATH}" || exit 1
else
    cp "${BINARY}" "${INSTALL_PATH}" || exit 1
    chmod +x "${INSTALL_PATH}" || exit 1
fi

echo "Installé avec succès dans ${INSTALL_PATH}."
echo "Test : babet --help"
