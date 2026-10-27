#!/bin/sh
# SPDX-License-Identifier: MIT
set -eu
cd "$(dirname "$0")/.."
env -u CFLAGS -u LDFLAGS -u MAKEFLAGS -u MFLAGS make clean all
make qh256-test sha256-test mapping-test
printf '\nBuilt: %s\n' "$PWD/build/k501-memory"
