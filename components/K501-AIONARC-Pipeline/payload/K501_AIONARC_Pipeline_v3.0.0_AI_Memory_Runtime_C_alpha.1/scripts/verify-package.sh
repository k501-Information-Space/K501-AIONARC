#!/bin/sh
# SPDX-License-Identifier: MIT
set -eu
cd "$(dirname "$0")/.."
if [ ! -f SHA256SUMS.txt ]; then
    echo 'SHA256SUMS.txt not found; package manifest is incomplete' >&2
    exit 2
fi
sha256sum -c SHA256SUMS.txt
make test
