#!/usr/bin/env python3
"""Check or apply clang-format across the repository.

Usage:
    python tools/check_format.py          # check (CI, pre-commit)
    python tools/check_format.py --fix    # rewrite files in place

The clang-format version matters: different majors format differently. CI pins
one version; install the same locally. If clang-format is missing, this exits
with a clear message instead of silently passing.
"""

from __future__ import annotations

import argparse
import pathlib
import shutil
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
SOURCE_SUFFIXES = {".h", ".hpp", ".hh", ".c", ".cpp", ".cc", ".cxx"}
SKIP_DIRECTORIES = {"build", ".git", "third_party", "out", "install"}


def find_sources() -> list[pathlib.Path]:
    files: list[pathlib.Path] = []
    for path in ROOT.rglob("*"):
        if not path.is_file() or path.suffix not in SOURCE_SUFFIXES:
            continue
        if any(part in SKIP_DIRECTORIES for part in path.relative_to(ROOT).parts):
            continue
        files.append(path)
    return sorted(files)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fix", action="store_true", help="apply formatting in place")
    args = parser.parse_args()

    clang_format = shutil.which("clang-format")
    if clang_format is None:
        print(
            "clang-format was not found on PATH.\n"
            "Windows: run through tools\\dev\\msvc_env.bat, which adds the LLVM "
            "shipped with Visual Studio.\n"
            "Linux: install clang-format (pin the same major version as CI).",
            file=sys.stderr,
        )
        return 2

    files = find_sources()
    if not files:
        print("No C/C++ source files found; nothing to check.")
        return 0

    command = [clang_format]
    command += ["-i"] if args.fix else ["--dry-run", "--Werror"]
    command += [str(path) for path in files]

    result = subprocess.run(command, cwd=ROOT, check=False)
    if result.returncode != 0 and not args.fix:
        print(
            "\nFormatting issues found. Run: python tools/check_format.py --fix",
            file=sys.stderr,
        )
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
