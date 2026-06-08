#!/usr/bin/env bash
set -euo pipefail

BINDIR="${HOME}/.local/bin"
mkdir -p build "$BINDIR"
cmake -S . -B build > /dev/null
make -C build -j"$(nproc)" kentc 2>&1 | grep -v "malloc_test"
cp build/kentc "$BINDIR/"
echo "installed kentc to $BINDIR/kentc"
