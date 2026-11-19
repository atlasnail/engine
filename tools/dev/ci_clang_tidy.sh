#!/usr/bin/env bash
# Run clang-tidy over the engine sources as a blocking check.
#
# Scope is engine code only (core, platform, and later modules). Tests and
# benchmarks are not analysed: they are allowed to be more direct, and analysing
# them doubles the run without catching product bugs.
#
# Findings are emitted as GitHub annotations because step logs need repository
# admin rights to read.
#
# Usage: ci_clang_tidy.sh <build-dir-with-compile-commands>

set -uo pipefail

build_dir="${1:?usage: ci_clang_tidy.sh <build-dir>}"

if [ ! -f "${build_dir}/compile_commands.json" ]; then
    echo "::error title=clang-tidy::no compile_commands.json in ${build_dir}; configure first"
    exit 1
fi

mapfile -t files < <(find engine -name '*.cpp' | sort)
if [ "${#files[@]}" -eq 0 ]; then
    echo "::error title=clang-tidy::no engine sources found"
    exit 1
fi

log="$(mktemp)"

if clang-tidy -p "$build_dir" --warnings-as-errors=* "${files[@]}" 2>&1 | tee "$log"; then
    rm -f "$log"
    exit 0
fi

echo "--- clang-tidy findings ---"
grep -E 'error:|warning:' "$log" || tail -n 20 "$log"

findings="$(grep -E -m10 'error:|warning:' "$log" || true)"
if [ -z "$findings" ]; then
    findings="$(tail -n 8 "$log")"
fi

while IFS= read -r line; do
    [ -n "$line" ] || continue
    echo "::error title=clang-tidy::${line}"
done <<< "$findings"

rm -f "$log"
exit 1
