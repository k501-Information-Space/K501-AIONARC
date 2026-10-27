# SPDX-License-Identifier: MIT
#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
if [ -x "$ROOT/build/k501-memory" ]; then BIN="$ROOT/build/k501-memory"; elif [ -x "$ROOT/bin/k501-memory-linux-x86_64" ]; then BIN="$ROOT/bin/k501-memory-linux-x86_64"; else echo 'Build first with make all' >&2; exit 2; fi
TMP=$(mktemp -d "k501-v3-quickstart.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/source/docs"
printf 'K501 proof before state runtime demo.\n' > "$TMP/source/README.md"
printf 'Temporal memory retains source identity and frame history.\n' > "$TMP/source/docs/memory.txt"
"$BIN" init "$TMP/store"
"$BIN" scan "$TMP/source" "$TMP/store" demo-root demo-001 none
"$BIN" verify "$TMP/store" demo-001
"$BIN" query "$TMP/store" latest 'temporal memory source identity'
printf '\nThis demonstration uses a disposable temporary store which is removed when the script exits.\n'
