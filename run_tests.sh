#!/bin/bash
# run_tests.sh — compile le projet puis teste les deux modes d'exécution
# (mode dossier et mode exécutable embarqué), avec un bilan global.
#
# Code de sortie : 0 si les deux modes passent, 1 sinon.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_NAME="luapilot"
TEST_DIR="${SCRIPT_DIR}/test"
EXAMPLES_DIR="${SCRIPT_DIR}/examples"

# --- 1. Compilation -------------------------------------------------
echo "### Compilation ###"
if ! bash "${SCRIPT_DIR}/build_local.sh"; then
    echo "ÉCHEC : la compilation a échoué."
    exit 1
fi
echo ""

BINARY="${TEST_DIR}/${PROJECT_NAME}"
if [ ! -f "${BINARY}" ]; then
    echo "ÉCHEC : binaire introuvable après compilation (${BINARY})."
    exit 1
fi

modes_ok=0
modes_total=0

# --- 2. Test mode dossier ------------------------------------------
echo "### Test 1/2 : mode dossier ###"
modes_total=$((modes_total + 1))

dir_output=$(cd "${TEST_DIR}" && ./"${PROJECT_NAME}" . __ARG__a __ARG__b 2>&1)
dir_rc=$?

if [ ${dir_rc} -eq 0 ]; then
    echo "${dir_output}" | grep -E 'PASS /' || true
    echo "  -> mode dossier : OK"
    modes_ok=$((modes_ok + 1))
else
    echo "--- sortie complète (mode dossier) ---"
    echo "${dir_output}"
    echo "--------------------------------------"
    echo "  -> mode dossier : ÉCHEC (code ${dir_rc})"
fi
echo ""

# --- 3. Test mode embarqué -----------------------------------------
echo "### Test 2/2 : mode embarqué ###"
modes_total=$((modes_total + 1))

ISOLATED_DIR=$(mktemp -d)
if [ -z "${ISOLATED_DIR}" ] || [ ! -d "${ISOLATED_DIR}" ]; then
    echo "  -> mode embarqué : ÉCHEC (impossible de créer un dossier temporaire)"
else
    EMBEDDED_BIN="${ISOLATED_DIR}/luapilot_embedded"

    # On construit l'exe embarqué à partir de examples/ (contenu Lua pur,
    # sans binaire dedans). Le résultat est placé SEUL dans un dossier isolé :
    # aucun main.lua / inspect.lua / mymod sur le disque à côté de lui.
    # S'il trouve ses fichiers, c'est forcément depuis le zip embarqué.
    if ! (cd "${SCRIPT_DIR}" && "${BINARY}" --create-exe "${EXAMPLES_DIR}" "${EMBEDDED_BIN}"); then
        echo "  -> mode embarqué : ÉCHEC (création de l'exécutable embarqué)"
    elif [ ! -f "${EMBEDDED_BIN}" ]; then
        echo "  -> mode embarqué : ÉCHEC (exécutable embarqué non produit)"
    else
        emb_output=$(cd "${ISOLATED_DIR}" && ./luapilot_embedded __ARG__a __ARG__b 2>&1)
        emb_rc=$?

        if [ ${emb_rc} -eq 0 ]; then
            echo "${emb_output}" | grep -E 'PASS /' || true
            echo "  -> mode embarqué : OK"
            modes_ok=$((modes_ok + 1))
        else
            echo "--- sortie complète (mode embarqué) ---"
            echo "${emb_output}"
            echo "---------------------------------------"
            echo "  -> mode embarqué : ÉCHEC (code ${emb_rc})"
        fi

        # Test 2bis : invocation via PATH, comme un binaire installé.
        # Reproduit "luapilot installé dans /usr/local/bin/ et invoqué depuis
        # ailleurs" — exactement le cas où argv[0] vaut juste le basename.
        # On lance par le nom seul (pas ./luapilot_embedded) avec ISOLATED_DIR
        # dans le PATH. Le cwd doit être writable pour que le harnais puisse
        # créer son sandbox, donc on utilise un autre tmpdir.
        PATH_TEST_CWD=$(mktemp -d)
        emb_path_output=$(cd "${PATH_TEST_CWD}" && PATH="${ISOLATED_DIR}:$PATH" luapilot_embedded __ARG__a __ARG__b 2>&1)
        emb_path_rc=$?
        rm -rf "${PATH_TEST_CWD}"

        if [ ${emb_path_rc} -eq 0 ]; then
            echo "${emb_path_output}" | grep -E 'PASS /' || true
            echo "  -> mode embarqué via PATH : OK"
        else
            echo "--- sortie complète (mode embarqué via PATH) ---"
            echo "${emb_path_output}"
            echo "------------------------------------------------"
            echo "  -> mode embarqué via PATH : ÉCHEC (code ${emb_path_rc})"
            # Annule le succès du test embarqué précédent : si le PATH ne
            # marche pas, le mode embarqué n'est pas vraiment fonctionnel.
            if [ ${emb_rc} -eq 0 ]; then
                modes_ok=$((modes_ok - 1))
            fi
        fi
    fi

    rm -rf "${ISOLATED_DIR}"
fi
echo ""

# --- 4. Bilan -------------------------------------------------------
echo "=========================================="
echo "Bilan : ${modes_ok}/${modes_total} modes OK"
echo "=========================================="

if [ ${modes_ok} -eq ${modes_total} ]; then
    exit 0
else
    exit 1
fi
