#!/bin/bash
# embed_lua_module.sh — transforme un fichier Lua en header C++
# contenant le source du module en raw string. Le header est destiné
# à être inclus pour enregistrer le module dans package.preload.
#
# Usage : embed_lua_module.sh <input.lua> <output.hpp> <module_name>
#
# Le <module_name> sert :
#   - à nommer la constante générée (INSPECT_LUA_SOURCE pour "inspect")
#   - à nommer le delimiter du raw string (LUA_INSPECT pour "inspect")
#     pour éviter toute collision si le code Lua contenait par hasard
#     la chaîne )" suivie d'un identifiant.

set -e

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <input.lua> <output.hpp> <module_name>"
    exit 1
fi

INPUT="$1"
OUTPUT="$2"
MODULE="$3"

if [ ! -f "${INPUT}" ]; then
    echo "Erreur : fichier source introuvable : ${INPUT}"
    exit 1
fi

# Nom de la constante en MAJUSCULES (inspect -> INSPECT_LUA_SOURCE)
# et tirets -> underscores au cas où.
CONST_NAME="$(echo "${MODULE}" | tr 'a-z-' 'A-Z_')_LUA_SOURCE"
DELIM="LUA_$(echo "${MODULE}" | tr 'a-z-' 'A-Z_')"
GUARD="EMBEDDED_$(echo "${MODULE}" | tr 'a-z-' 'A-Z_')_HPP"

# Garde-fou : si le fichier Lua contient la séquence )DELIM" elle casserait
# le raw string. C'est extrêmement improbable mais on vérifie quand même.
if grep -q ")${DELIM}\"" "${INPUT}"; then
    echo "Erreur : le fichier ${INPUT} contient la séquence delimiter '${DELIM}',"
    echo "qui casserait le raw string C++. Changer le nom du module ou éditer le fichier."
    exit 1
fi

mkdir -p "$(dirname "${OUTPUT}")"

{
    echo "// Généré automatiquement depuis ${INPUT} par embed_lua_module.sh."
    echo "// Ne PAS éditer à la main : régénéré à chaque build."
    echo "#ifndef ${GUARD}"
    echo "#define ${GUARD}"
    echo ""
    echo "inline constexpr const char ${CONST_NAME}[] = R\"${DELIM}("
    cat "${INPUT}"
    echo ")${DELIM}\";"
    echo ""
    echo "#endif // ${GUARD}"
} > "${OUTPUT}"

echo "Module ${MODULE} embarqué : ${INPUT} -> ${OUTPUT}"
