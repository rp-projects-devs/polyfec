#!/usr/bin/env bash
# Tests de bout en bout de la ligne de commande.
# Utilisation : tests/cli_tests.sh [chemin/vers/polyfec]

set -u

BIN="${1:-./polyfec}"
DATA_DIR="$(dirname "$0")/data"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

passed=0
failed=0

ok() {
    echo "[OK] $1"
    passed=$((passed + 1))
}

fail() {
    echo "[FAIL] $1"
    failed=$((failed + 1))
}

# expect_output <nom> <sortie attendue> <commande...>
expect_output() {
    local name="$1" expected="$2"
    shift 2
    local actual
    actual="$("$@" 2>/dev/null)"
    if [ "$actual" = "$expected" ]; then ok "$name"; else fail "$name (obtenu : $actual)"; fi
}

# expect_failure <nom> <commande...> : la commande doit echouer
expect_failure() {
    local name="$1"
    shift
    if "$@" >/dev/null 2>&1; then fail "$name"; else ok "$name"; fi
}

zeros() {
    printf '%*s' "$1" '' | tr ' ' '0'
}

if [ ! -x "$BIN" ]; then
    echo "Executable introuvable : $BIN (lancer make d'abord)"
    exit 1
fi

echo "== Manipulation de bits (position 0 = bit le plus a droite)"
expect_output "read 0 101010" "0" "$BIN" read 0 101010
expect_output "read 1 101010" "1" "$BIN" read 1 101010
expect_output "set 0 101010" "101011" "$BIN" set 0 101010
expect_output "set 1 101010 (deja a 1)" "101010" "$BIN" set 1 101010
expect_output "mod 0 101010" "101011" "$BIN" mod 0 101010
expect_output "mod 1 101010" "101000" "$BIN" mod 1 101010
expect_output "error 0 ne modifie pas le mot" "1010001010011011" "$BIN" error 0 1010001010011011

echo "== Encodage, decodage, correction"
info="10100010"
code_hex="$("$BIN" code 0x11D "$info")"
code_bin="$("$BIN" code 100011101 "$info")"

if [ "${#code_hex}" -eq 256 ]; then ok "code produit un mot de 256 bits"; else fail "code produit un mot de 256 bits"; fi
if [ "$code_hex" = "$code_bin" ]; then ok "polynome en hexadecimal ou en binaire : meme resultat"; else fail "polynome en hexadecimal ou en binaire"; fi

expect_output "decode(code(m)) = m complete a 240 bits" "$info$(zeros 232)" "$BIN" decode 0x11D "$code_hex"
expect_output "corrige un mot sans erreur : inchange" "$code_hex" "$BIN" corrige 0x11D "$code_hex"

for pos in 0 7 8 100 247 254; do
    damaged="$("$BIN" mod "$pos" "$code_hex")"
    expect_output "corrige une erreur en position $pos" "$code_hex" "$BIN" corrige 0x11D "$damaged"
done

echo "== Transmission de fichiers avec un taux nul"
"$BIN" "$DATA_DIR/message.txt" 0 > "$TMP_DIR/message.out"
if cmp -s "$DATA_DIR/message.txt" "$TMP_DIR/message.out"; then ok "fichier texte restitue a l'identique"; else fail "fichier texte restitue a l'identique"; fi

"$BIN" "$BIN" 0 > "$TMP_DIR/binary.out"
if cmp -s "$BIN" "$TMP_DIR/binary.out"; then ok "fichier binaire restitue a l'identique"; else fail "fichier binaire restitue a l'identique"; fi

: > "$TMP_DIR/empty"
expect_output "fichier vide" "" "$BIN" "$TMP_DIR/empty" 0

echo "== Recherche de polynomes"
count="$("$BIN" search4poly | grep -c '^0x')"
if [ "$count" -eq 16 ]; then ok "search4poly trouve 16 polynomes generateurs"; else fail "search4poly trouve 16 polynomes (obtenu : $count)"; fi

echo "== Entrees invalides"
expect_failure "taux superieur a 1" "$BIN" error 2 1010
expect_failure "taux non numerique" "$BIN" "$DATA_DIR/message.txt" abc
expect_failure "mot non binaire" "$BIN" read 0 10a1
expect_failure "position hors du mot" "$BIN" read 10 1010
expect_failure "decode sur un mot de mauvaise taille" "$BIN" decode 0x11D 1010
expect_failure "corrige avec un polynome non generateur" "$BIN" corrige 0x101 "$code_hex"
expect_failure "fichier inexistant" "$BIN" "$TMP_DIR/inexistant" 0
expect_failure "commande inconnue" "$BIN" inconnue a b c

echo
echo "$passed test(s) reussi(s), $failed echec(s)."
[ "$failed" -eq 0 ]
