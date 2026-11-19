#!/usr/bin/env bash
# Build a CMake preset, and on failure emit the compiler errors as GitHub
# annotations.
#
# Workflow step logs need repository admin rights to read; annotations do not.
#
# Usage: ci_build.sh <preset>

set -uo pipefail

preset="${1:?usage: ci_build.sh <preset>}"
log="$(mktemp)"

if cmake --build --preset "$preset" 2>&1 | tee "$log"; then
    rm -f "$log"
    exit 0
fi

echo "--- build output (tail) ---"
tail -n 60 "$log"

errors="$(grep -E -m10 'error:|FAILED:' "$log" || true)"
if [ -z "$errors" ]; then
    errors="$(tail -n 8 "$log")"
fi

while IFS= read -r line; do
    [ -n "$line" ] || continue
    echo "::error title=cmake --build ${preset} failed::${line}"
done <<< "$errors"

rm -f "$log"
exit 1
