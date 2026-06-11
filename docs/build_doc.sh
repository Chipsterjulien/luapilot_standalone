#!/usr/bin/env bash
# build-docs.sh — generate the LuaPilot manual PDFs from per-language
# Markdown sources, auto-detecting which languages are present.
#
# Discovery is driven by docs/manual_order_<lang>.txt files :
# every such file is taken as evidence that docs/<lang>/ exists and
# wants a PDF built. Add a new language by creating
# docs/<newlang>/... + docs/manual_order_<newlang>.txt — this script
# will pick it up automatically.
#
# Usage :
#   ./build-docs.sh                  # build every detected language
#   ./build-docs.sh en               # build only EN
#   ./build-docs.sh en fr            # build EN and FR explicitly
#   ./build-docs.sh --clean          # remove all generated PDFs
#   ./build-docs.sh --list           # list detected languages, build nothing
#   ./build-docs.sh -h | --help      # show help
#
# Requirements : pandoc + a LaTeX engine (xelatex, lualatex, or pdflatex).
# xelatex is recommended for proper UTF-8 / accent handling.
#
# Exit codes :
#   0  — at least one PDF was built successfully (or --clean/--list ran)
#   1  — usage error (unknown flag, etc.)
#   2  — prerequisite missing (pandoc or LaTeX engine not found)
#   3  — at least one build failed

set -u

# ---- Resolve script location, regardless of where it's called from --
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# ---- Defaults --------------------------------------------------------
PDF_ENGINE_DEFAULT="xelatex"
PDF_ENGINE="${PDF_ENGINE:-$PDF_ENGINE_DEFAULT}"

# ---- Helpers ---------------------------------------------------------
die() {
    echo "build-docs: $1" >&2
    exit "${2:-1}"
}

show_help() {
    sed -n '2,/^$/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

# Look up a human-friendly title for a language code. Falls back to a
# generic title for codes we don't know. Add cases as new languages
# are added.
title_for_lang() {
    case "$1" in
        en) echo "LuaPilot Manual" ;;
        fr) echo "Manuel LuaPilot" ;;
        de) echo "LuaPilot Handbuch" ;;
        es) echo "Manual LuaPilot" ;;
        it) echo "Manuale LuaPilot" ;;
        pt) echo "Manual do LuaPilot" ;;
        *)  echo "LuaPilot Manual ($1)" ;;
    esac
}

# Read the ordered list of files from manual_order_<lang>.txt,
# stripping comments and blank lines. Paths are relative to REPO_ROOT.
read_order_file() {
    local order="$1"
    grep -vE '^\s*(#|$)' "$order" || true
}

# Discover languages from the manual_order_*.txt files in SCRIPT_DIR.
# Prints each detected lang code on its own line. A language is only
# returned if its corresponding docs/<lang>/ directory exists too.
discover_languages() {
    local order lang
    for order in "$SCRIPT_DIR"/manual_order_*.txt; do
        [ -f "$order" ] || continue
        lang="$(basename "$order" .txt)"
        lang="${lang#manual_order_}"
        if [ ! -d "$SCRIPT_DIR/$lang" ]; then
            echo "build-docs: warning: $order exists but $SCRIPT_DIR/$lang/ does not, skipping '$lang'" >&2
            continue
        fi
        echo "$lang"
    done
}

# Check that all required tools are present.
check_prereqs() {
    command -v pandoc >/dev/null 2>&1 || die "pandoc not found (install: pacman -S pandoc / apt install pandoc)" 2
    command -v "$PDF_ENGINE" >/dev/null 2>&1 || \
        die "PDF engine '$PDF_ENGINE' not found (install: pacman -S texlive-xetex / apt install texlive-xetex). Override with PDF_ENGINE=lualatex or pdflatex." 2
}

# Build a single language. Returns 0 on success, non-zero on failure.
build_one() {
    local lang="$1"
    local order="$SCRIPT_DIR/manual_order_$lang.txt"
    local out="$SCRIPT_DIR/manual-$lang.pdf"

    if [ ! -f "$order" ]; then
        echo "build-docs: no order file for '$lang' ($order), skipping" >&2
        return 1
    fi

    # Collect the files to feed pandoc, in order. Read into an array
    # so that paths with spaces would still work (though the project
    # doesn't have any).
    local files=()
    local line
    while IFS= read -r line; do
        files+=("$line")
    done < <(read_order_file "$order")

    if [ ${#files[@]} -eq 0 ]; then
        echo "build-docs: $order has no active files, skipping '$lang'" >&2
        return 1
    fi

    local title
    title="$(title_for_lang "$lang")"

    echo "Building $out  ($lang, ${#files[@]} files)"

    # pandoc reads paths relative to its CWD, so we run from REPO_ROOT
    # — the same convention used by manual_order_<lang>.txt itself.
    (
        cd "$REPO_ROOT" || exit 1
        pandoc "${files[@]}" \
            --toc \
            --pdf-engine="$PDF_ENGINE" \
            --metadata title="$title" \
            --metadata lang="$lang" \
            --variable=geometry:margin=2cm \
            --variable=colorlinks:true \
            --variable=linkcolor:blue \
            --variable=urlcolor:blue \
            --variable=monofont:'DejaVu Sans Mono' \
            -o "$out"
    )
}

# ---- Argument parsing ------------------------------------------------
WANT_CLEAN=0
WANT_LIST=0
EXPLICIT_LANGS=()

while [ $# -gt 0 ]; do
    case "$1" in
        -h|--help)
            show_help
            exit 0
            ;;
        --clean)
            WANT_CLEAN=1
            shift
            ;;
        --list)
            WANT_LIST=1
            shift
            ;;
        -*)
            die "unknown option: $1 (try --help)" 1
            ;;
        *)
            EXPLICIT_LANGS+=("$1")
            shift
            ;;
    esac
done

# ---- Modes -----------------------------------------------------------
if [ "$WANT_CLEAN" -eq 1 ]; then
    removed=0
    for pdf in "$SCRIPT_DIR"/manual-*.pdf; do
        [ -f "$pdf" ] || continue
        rm -f "$pdf"
        echo "removed: $pdf"
        removed=$((removed + 1))
    done
    echo "$removed PDF(s) removed."
    exit 0
fi

if [ "$WANT_LIST" -eq 1 ]; then
    echo "Detected languages :"
    while IFS= read -r lang; do
        [ -n "$lang" ] && echo "  $lang ($(title_for_lang "$lang"))"
    done < <(discover_languages)
    exit 0
fi

# ---- Build mode ------------------------------------------------------
check_prereqs

if [ ${#EXPLICIT_LANGS[@]} -gt 0 ]; then
    languages=("${EXPLICIT_LANGS[@]}")
else
    languages=()
    while IFS= read -r lang; do
        [ -n "$lang" ] && languages+=("$lang")
    done < <(discover_languages)
fi

if [ ${#languages[@]} -eq 0 ]; then
    die "no languages to build (looked for $SCRIPT_DIR/manual_order_*.txt)" 1
fi

ok=0
fail=0
for lang in "${languages[@]}"; do
    if build_one "$lang"; then
        ok=$((ok + 1))
    else
        fail=$((fail + 1))
    fi
done

echo
echo "=========================================="
echo "Built : $ok    Failed : $fail    Total : ${#languages[@]}"
echo "=========================================="

if [ "$fail" -gt 0 ]; then
    exit 3
fi
