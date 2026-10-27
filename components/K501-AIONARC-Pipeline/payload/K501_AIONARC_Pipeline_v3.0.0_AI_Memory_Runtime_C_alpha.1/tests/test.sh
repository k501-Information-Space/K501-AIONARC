#!/bin/bash
# SPDX-License-Identifier: MIT
set -euo pipefail
BIN=${1:-"$(cd "$(dirname "$0")/.." && pwd)/build/k501-memory"}
PKG_ROOT=$(cd "$(dirname "$0")/.." && pwd)
TMP=$(mktemp -d "k501-v3a1-test.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
SRC="$TMP/source corpus"
STORE="$TMP/memory store"
mkdir -p "$SRC/docs with spaces" "$SRC/config"
cat > "$SRC/README.md" <<'TXT'
K501 AI Memory Runtime proof before state.
A source record is not the same as derived meaning.
TXT
cat > "$SRC/docs with spaces/Übersicht.txt" <<'TXT'
Object identity depends on root, path and type.
History must remain append-only. Verify before retrieval.
TXT
printf '{"proof_before_state":true,"runtime":"K501"}\n' > "$SRC/config/runtime.json"
printf '\001\002\003\377' > "$SRC/binary.dat"
ln -s 'docs with spaces/Übersicht.txt' "$SRC/document-link"

"$BIN" qh256-test | grep -q 'QH256 CORE TEST PASS'
"$BIN" sha256-test | grep -q 'SHA256 TEST PASS'
"$BIN" mapping-test | grep -q 'QH256 MAPPING PROFILE V3A1 TEST PASS'
"$BIN" init "$STORE" | grep -q 'INIT PASS'
"$BIN" scan "$SRC" "$STORE" k501-test-root frame-001 none | grep -q 'proof_before_state=PASS'
"$BIN" verify "$STORE" frame-001 | grep -q 'VERIFY PASS'
"$BIN" verify-source "$STORE" frame-001 "$SRC" | grep -q 'VERIFY SOURCE PASS'
"$BIN" query "$STORE" latest 'proof before state' | grep -q 'PATH=README.md'
"$BIN" query "$STORE" latest 'object identity path type' | grep -q 'Übersicht.txt'
ID=$(awk -F '\t' '$1=="O" && $10=="UkVBRE1FLm1k" {print $2; exit}' "$STORE/frames/frame-001/STATE.tsv")
test -n "$ID"
"$BIN" trace "$STORE" latest "$ID" | grep -q "object_id=$ID"
"$BIN" history "$STORE" "$ID" | grep -q 'FRAMES.*1'
"$BIN" diff "$STORE" frame-001 | grep -q 'event_count'
printf '\nUpdated README content for frame two.\n' >> "$SRC/README.md"
printf 'additional object for transition test\n' > "$SRC/new-note.txt"
"$BIN" scan "$SRC" "$STORE" k501-test-root frame-002 frame-001 | grep -q 'proof_before_state=PASS'
"$BIN" verify "$STORE" frame-002 | grep -q 'VERIFY PASS'
"$BIN" verify "$STORE" frame-001 | grep -q 'VERIFY PASS'
"$BIN" verify-source "$STORE" frame-002 "$SRC" | grep -q 'VERIFY SOURCE PASS'
"$BIN" history "$STORE" "$ID" | grep -q 'FRAMES.*2'
"$BIN" diff "$STORE" frame-002 | grep -q 'CONTENT_CHANGED'
if "$BIN" scan "$SRC" "$STORE" k501-test-root frame-002 latest >/dev/null 2>&1; then echo 'FAIL: reused frame ID was accepted' >&2; exit 1; fi

TAMPER="$TMP/tampered store"
cp -a "$STORE" "$TAMPER"
printf '\n' >> "$TAMPER/frames/frame-002/INDEX.tsv"
if "$BIN" verify "$TAMPER" frame-002 >/dev/null 2>&1; then echo 'FAIL: tampered index was accepted' >&2; exit 1; fi
DRIFT="$TMP/drift corpus"
cp -a "$SRC" "$DRIFT"
"$BIN" verify-source "$STORE" frame-002 "$DRIFT" | grep -q 'VERIFY SOURCE PASS'
printf 'local source edit\n' >> "$DRIFT/README.md"
if "$BIN" verify-source "$STORE" frame-002 "$DRIFT" >/dev/null 2>&1; then echo 'FAIL: source drift was accepted' >&2; exit 1; fi
BRANCH="$TMP/branch store"
"$BIN" init "$BRANCH" >/dev/null
"$BIN" scan "$SRC" "$BRANCH" branch-root b0 none >/dev/null
"$BIN" scan "$SRC" "$BRANCH" branch-root b1 b0 >/dev/null
"$BIN" scan "$SRC" "$BRANCH" branch-root b2 b0 >/dev/null
if "$BIN" latest "$BRANCH" >/dev/null 2>&1; then echo 'FAIL: ambiguous branch tip was silently selected' >&2; exit 1; fi
if "$BIN" scan "$SRC" "$STORE" k501-test-root overlap-001 none >/dev/null 2>&1; then echo 'FAIL: parent=none on populated store was accepted' >&2; exit 1; fi

# Regression G: refuse Alpha 0 store schema rather than silently reinterpret it.
LEGACY_STORE="$TMP/legacy-v3a0-store"
mkdir -p "$LEGACY_STORE/frames"
printf 'K501-AIONARC-AI-MEMORY-STORE-V3A0\n' > "$LEGACY_STORE/STORE_FORMAT.txt"
if "$BIN" latest "$LEGACY_STORE" >/dev/null 2>&1; then
    echo 'FAIL: Alpha 1 accepted an Alpha 0 store marker' >&2
    exit 1
fi

# Regression H: reject both source-inside-store and store-inside-source overlap.
OVERLAP_SRC="$TMP/source-containing-store"
OVERLAP_STORE="$OVERLAP_SRC/store"
mkdir -p "$OVERLAP_SRC"
printf 'overlap probe\n' > "$OVERLAP_SRC/README.md"
"$BIN" init "$OVERLAP_STORE" >/dev/null
if "$BIN" scan "$OVERLAP_SRC" "$OVERLAP_STORE" overlap-root overlap-store-inside-source none >/dev/null 2>&1; then
    echo 'FAIL: store nested inside source was accepted' >&2
    exit 1
fi
PARENT_STORE="$TMP/store-containing-source"
INNER_SRC="$PARENT_STORE/source"
"$BIN" init "$PARENT_STORE" >/dev/null
mkdir -p "$INNER_SRC"
printf 'inside-store source probe\n' > "$INNER_SRC/README.md"
if "$BIN" scan "$INNER_SRC" "$PARENT_STORE" overlap-root overlap-source-inside-store none >/dev/null 2>&1; then
    echo 'FAIL: source nested inside store was accepted' >&2
    exit 1
fi

# Regression A: invalid UTF-8 is rejected as text, stored structurally, then verifies.
MAP_SRC="$TMP/mapping-invalid-utf8"
MAP_STORE="$TMP/store-invalid-utf8"
mkdir -p "$MAP_SRC"
printf 'ordinary searchable baseline\n' > "$MAP_SRC/README.md"
printf 'invalid byte: \377\n' > "$MAP_SRC/invalid-utf8.txt"
"$BIN" init "$MAP_STORE" >/dev/null
"$BIN" scan "$MAP_SRC" "$MAP_STORE" mapping-root mapping-utf8 none | grep -q 'proof_before_state=PASS'
grep -F 'INVALID_UTF8' "$MAP_STORE/frames/mapping-utf8/INDEX.tsv" >/dev/null
"$BIN" verify "$MAP_STORE" mapping-utf8 | grep -q 'VERIFY PASS'

# Regression B: UTF-8-valid embedded NUL is distinguished and frame verifies.
MAP_SRC="$TMP/mapping-embedded-nul"
MAP_STORE="$TMP/store-embedded-nul"
mkdir -p "$MAP_SRC"
printf 'ordinary searchable baseline\n' > "$MAP_SRC/README.md"
printf 'before\000after\n' > "$MAP_SRC/embedded-nul.txt"
"$BIN" init "$MAP_STORE" >/dev/null
"$BIN" scan "$MAP_SRC" "$MAP_STORE" mapping-root mapping-nul none | grep -q 'proof_before_state=PASS'
grep -F 'EMBEDDED_NUL' "$MAP_STORE/frames/mapping-nul/INDEX.tsv" >/dev/null
"$BIN" verify "$MAP_STORE" mapping-nul | grep -q 'VERIFY PASS'

# Regression C: captured-empty is a real result, serializes an empty Base64 field and round-trips.
MAP_SRC="$TMP/mapping-empty"
MAP_STORE="$TMP/store-empty"
mkdir -p "$MAP_SRC"
printf 'ordinary searchable baseline\n' > "$MAP_SRC/README.md"
: > "$MAP_SRC/empty.txt"
"$BIN" init "$MAP_STORE" >/dev/null
"$BIN" scan "$MAP_SRC" "$MAP_STORE" mapping-root mapping-empty none | grep -q 'proof_before_state=PASS'
grep -F 'CAPTURED_EMPTY' "$MAP_STORE/frames/mapping-empty/INDEX.tsv" >/dev/null
if ! grep -E $'\t0\t$' "$MAP_STORE/frames/mapping-empty/TEXT.tsv" >/dev/null; then
    echo 'FAIL: empty text row lost its final empty Base64 field' >&2
    exit 1
fi
"$BIN" verify "$MAP_STORE" mapping-empty | grep -q 'VERIFY PASS'

# Regression D: file-size limit records the candidate without confusing it with invalid UTF-8.
MAP_SRC="$TMP/mapping-file-limit"
MAP_STORE="$TMP/store-file-limit"
mkdir -p "$MAP_SRC"
printf 'ordinary searchable baseline\n' > "$MAP_SRC/README.md"
truncate -s 16777217 "$MAP_SRC/large.txt"
"$BIN" init "$MAP_STORE" >/dev/null
"$BIN" scan "$MAP_SRC" "$MAP_STORE" mapping-root mapping-limit none | grep -q 'proof_before_state=PASS'
grep -F 'SKIPPED_FILE_LIMIT' "$MAP_STORE/frames/mapping-limit/INDEX.tsv" >/dev/null
"$BIN" verify "$MAP_STORE" mapping-limit | grep -q 'VERIFY PASS'

# Regression E: no PDF extractor must not block structural frame/verify/other text search.
PDF_SRC="$TMP/pdf-no-extractor"
PDF_STORE="$TMP/store-pdf-no-extractor"
NO_TOOLS="$TMP/no-tools"
mkdir -p "$PDF_SRC" "$NO_TOOLS"
printf 'K501 CORE SURVIVES MISSING OPTIONAL PDF EXTRACTOR.\n' > "$PDF_SRC/README.md"
cp "$PKG_ROOT/tests/fixtures/minimal-text.pdf" "$PDF_SRC/adapter.pdf"
"$BIN" init "$PDF_STORE" >/dev/null
env PATH="$NO_TOOLS" "$BIN" scan "$PDF_SRC" "$PDF_STORE" pdf-root pdf-missing none | grep -q 'proof_before_state=PASS'
grep -F 'PDF_EXTRACTOR_UNAVAILABLE_OR_FAILED' "$PDF_STORE/frames/pdf-missing/INDEX.tsv" >/dev/null
"$BIN" verify "$PDF_STORE" pdf-missing | grep -q 'VERIFY PASS'
env PATH="$NO_TOOLS" "$BIN" query "$PDF_STORE" latest 'CORE SURVIVES' | grep -q 'PATH=README.md'

# Regression F: available PDF extractor (when installed) is a true successful-capture case.
if command -v pdftotext >/dev/null 2>&1; then
    PDF_SRC="$TMP/pdf-with-extractor"
    PDF_STORE="$TMP/store-pdf-with-extractor"
    mkdir -p "$PDF_SRC"
    printf 'K501 CORE SURVIVES MISSING OPTIONAL PDF EXTRACTOR.\n' > "$PDF_SRC/README.md"
    cp "$PKG_ROOT/tests/fixtures/minimal-text.pdf" "$PDF_SRC/adapter.pdf"
    "$BIN" init "$PDF_STORE" >/dev/null
    "$BIN" scan "$PDF_SRC" "$PDF_STORE" pdf-root pdf-present none | grep -q 'proof_before_state=PASS'
    grep -F 'CAPTURED' "$PDF_STORE/frames/pdf-present/INDEX.tsv" | grep -F 'pdftotext-layout-utf8-v1' >/dev/null
    "$BIN" verify "$PDF_STORE" pdf-present | grep -q 'VERIFY PASS'
    "$BIN" query "$PDF_STORE" latest 'PDF ADAPTER SENTINEL' | grep -q 'PATH=adapter.pdf'
fi

printf 'K501 V3A1 TEST SUITE: PASS\n'
printf 'checks: QH256 core; SHA-256 known vectors; mapping-profile truth table; initial and subsequent frames; proof-before-state; index/status reconstruction; empty-text roundtrip; UTF-8/NUL distinction; file limit; PDF adapter available/unavailable; exact lexical retrieval; trace/history/diff; source parity and drift; tamper/repeated-ID/branch/legacy-schema refusal; both overlap directions\n'
