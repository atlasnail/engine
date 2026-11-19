# Release checklist

**Status: stub.** Complete this list during M9 and keep it exact; every release follows it.

## Pre-release

- [ ] Version bumped in one place; `CHANGELOG.md` complete with migration notes.
- [ ] All P0 milestone items closed; no known P0 bugs.
- [ ] `docs/status.md`, `docs/roadmap.md`, ADR statuses and `AGENTS.md` match the code.
- [ ] Public API audited: every exported symbol classified stable/experimental/internal and documented.
- [ ] Save/asset format versions and migrations tested from every supported old version.
- [ ] Full benchmark suite run on Tier A for Windows and Linux; records committed; results within targets or exceptions documented.
- [ ] 24 h soak passed on both platforms; no leaks, underruns or crashes.
- [ ] ASan/UBSan and MSVC leak checks clean; fuzz corpus run.
- [ ] Example game builds and runs against the installed SDK on a clean machine per platform.
- [ ] Dependencies: versions pinned, licenses present, no known critical CVEs.
- [ ] Reproducible build verified (same commit + toolchain → same artifacts, or documented differences).

## Packaging

- [ ] Windows package includes headers, libs, CMake config, licenses, README; tested from a clean path with spaces and non-ASCII characters.
- [ ] Linux tarball includes the same; tested on a clean distro install with only documented dependencies.
- [ ] Debug symbols archived separately; no secrets, dumps or build caches in the package.
- [ ] Sample project or template builds with `find_package` and no source-tree access.

## Post-release

- [ ] Tag pushed; release notes published with known issues and workarounds.
- [ ] Install instructions verified by someone other than the author (or a documented self-check from a fresh VM).
- [ ] Rollback plan documented (previous tag + save compatibility statement).
