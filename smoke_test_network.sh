#!/usr/bin/env bash
# smoke_test_network.sh — Tests d'intégration réseau pour Babet.
#
# À lancer MANUELLEMENT avant chaque release tag, jamais par
# run_tests.sh : ce dernier doit rester hermétique/offline pour
# tourner partout (CI isolée, build chroot, machine sans réseau,
# etc.). Ce script-ci sert à attraper les bugs que le harness
# offline ne peut pas voir.
#
# Cas d'usage qui a motivé la création : le bug TLS de v1.6.0 (CA
# bundle introuvable avec OpenSSL vendored compilé sans
# --openssldir, fixé en v1.6.1). Les 799 tests du harness principal
# ne l'ont pas détecté parce qu'ils utilisent tous des certs
# auto-signés avec ca_cert explicite — exactement le chemin qui
# court-circuite le bug.
#
# Stratégie : on neutralise SSL_CERT_FILE et SSL_CERT_DIR via
# env -u pour s'assurer que le binaire trouve le CA bundle TOUT
# SEUL, via ses propres mécanismes (compile-time --openssldir
# + probing runtime).

set -e

BIN="${1:-./test/babet}"
if [ ! -x "$BIN" ]; then
    echo "Usage: $0 [chemin/vers/babet]"
    echo "Binaire introuvable ou non exécutable : $BIN"
    exit 1
fi

# Working dir temporaire avec cleanup automatique sur exit (même
# en cas d'erreur), pour ne pas laisser traîner /tmp/babet-*/.
TMPDIR=$(mktemp -d -t babet-smoke-XXXXXX)
trap 'rm -rf "$TMPDIR"' EXIT

PASS=0
FAIL=0

# Helper : écrit le script Lua dans $TMPDIR/main.lua, lance le
# binaire en mode dossier sur $TMPDIR, et compare la sortie à un
# pattern grep -E.
#
# env -u SSL_CERT_FILE -u SSL_CERT_DIR : on neutralise ces
# variables pour que le binaire doive se débrouiller sans aide
# externe. Si ces variables étaient honorées au lieu du probing,
# on raterait le bug qu'on veut attraper.
run_test() {
    local name="$1"
    local script="$2"
    local expected_pattern="$3"

    echo "$script" > "$TMPDIR/main.lua"

    local output
    output=$(env -u SSL_CERT_FILE -u SSL_CERT_DIR \
             "$BIN" "$TMPDIR" 2>&1) || true

    if echo "$output" | grep -qE "$expected_pattern"; then
        echo "[PASS] $name"
        PASS=$((PASS + 1))
    else
        echo "[FAIL] $name"
        echo "       attendu (pattern) : $expected_pattern"
        echo "       reçu              : $output"
        FAIL=$((FAIL + 1))
    fi
}

echo "=== Smoke tests réseau (HTTPS, sans SSL_CERT_FILE/DIR) ==="
echo "binaire : $BIN"
echo "tmpdir  : $TMPDIR"
echo

# -------------------------------------------------------------------
# Test 1 : HTTPS sur un site public stable, verify=true par défaut.
# Cible : google.com (HTTP/HTTPS depuis ~toujours, cert Let's Encrypt
# ou Google Trust Services, chaîne CA standard présente dans tous
# les trust stores).
# Échec attendu sans le fix v1.6.1 : "ERR=tls: certificate verify
# failed: unable to get local issuer certificate" parce que le CA
# bundle n'est pas trouvé.
# -------------------------------------------------------------------
run_test "HTTPS google.com verify=true OK sans ca_cert" \
'local r, e = babet.http.request{ url = "https://www.google.com/" }
if r then print("STATUS=" .. r.status) else print("ERR=" .. tostring(e)) end' \
'STATUS=(2|3)[0-9][0-9]'

# -------------------------------------------------------------------
# Test 2 : HTTPS sur AUR (cas concret qui a motivé le fix).
# Endpoint API : retourne du JSON, status 200 attendu sur une
# requête bien formée. Si l'AUR change son schéma, ce test peut
# devoir être ajusté — c'est le risque assumé d'un smoke test
# contre un vrai service.
# -------------------------------------------------------------------
run_test "HTTPS AUR API OK sans ca_cert (cas yaourt)" \
'local r, e = babet.http.request{
    url = "https://aur.archlinux.org/rpc/v5/info?arg[]=google-chrome"
}
if r then print("STATUS=" .. r.status) else print("ERR=" .. tostring(e)) end' \
'STATUS=200'

# -------------------------------------------------------------------
# Test 3 : badssl.com sert un cert expiré sur expired.badssl.com.
# verify=true (défaut) doit le rejeter avec une erreur lisible.
# On vérifie juste qu'on obtient "ERR=" (donc PAS de STATUS), peu
# importe le wording exact du message d'erreur OpenSSL.
# -------------------------------------------------------------------
run_test "HTTPS expired cert rejeté par verify=true" \
'local r, e = babet.http.request{ url = "https://expired.badssl.com/" }
if r then print("UNEXPECTED_OK=" .. r.status)
else print("ERR=" .. tostring(e)) end' \
'^ERR='

# -------------------------------------------------------------------
# Test 4 : même endpoint cert expiré, mais verify=false.
# Le bypass volontaire doit fonctionner et retourner un 200.
#
# Pourquoi ce 4e test alors que le 3 suffit à valider verify=true :
# c'est un test de SYMÉTRIE. Sans lui, si un refactor casse
# silencieusement le chemin verify=false (par exemple un check
# "verify forcé" qui se glisse par erreur dans la logique), on
# ne s'en rendrait compte qu'en prod. Coût marginal nul, valeur
# de non-régression nette.
# -------------------------------------------------------------------
run_test "HTTPS expired cert accepté avec verify=false" \
'local r, e = babet.http.request{
    url = "https://expired.badssl.com/", verify = false
}
if r then print("STATUS=" .. r.status) else print("ERR=" .. tostring(e)) end' \
'STATUS=(2|3)[0-9][0-9]'

echo
echo "=========================================="
echo "Résultat : $PASS PASS / $FAIL FAIL"
echo "=========================================="

if [ "$FAIL" -gt 0 ]; then
    exit 1
fi
