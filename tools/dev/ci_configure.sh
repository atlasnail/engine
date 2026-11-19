#!/usr/bin/env bash
# Configure a CMake preset, and on failure emit the reason as a GitHub
# annotation.
#
# Why: workflow step logs are only readable by repository admins, so a plain
# "exit code 1" gives nobody enough to act on. Annotations are visible to
# anyone who can read the repository.
#
# Usage: ci_configure.sh <preset> [extra cmake args...]

set -uo pipefail

preset="${1:?usage: ci_configure.sh <preset> [args...]}"
shift || true

log="$(mktemp)"

if cmake --preset "$preset" "$@" 2>&1 | tee "$log"; then
    rm -f "$log"
    exit 0
fi

# Show the tail in the step output too, for anyone who can open it.
echo "--- configure output (tail) ---"
tail -n 40 "$log"

# Annotations are limited to ten per step; prefer the first real error block.
block="$(grep -n -m1 -A6 -E 'CMake Error|CMake Warning \(dev\)' "$log" || true)"
if [ -z "$block" ]; then
    block="$(tail -n 8 "$log")"
fi

while IFS= read -r line; do
    [ -n "$line" ] || continue
    echo "::error title=cmake --preset ${preset} failed::${line}"
done <<< "$block"

rm -f "$log"
exit 1
