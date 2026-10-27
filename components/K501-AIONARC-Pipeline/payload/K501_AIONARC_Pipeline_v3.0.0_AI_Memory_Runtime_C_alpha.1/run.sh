#!/bin/sh
# SPDX-License-Identifier: MIT
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if [ -x "$ROOT/build/k501-memory" ]; then
    exec "$ROOT/build/k501-memory" "$@"
fi
if [ -x "$ROOT/bin/k501-memory-linux-x86_64" ]; then
    exec "$ROOT/bin/k501-memory-linux-x86_64" "$@"
fi
echo 'K501 V3A1 executable not built. Run: make all' >&2
exit 2
