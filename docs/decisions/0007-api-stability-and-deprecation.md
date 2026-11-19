# ADR-0007: API stability tiers and deprecation policy

**Status:** Accepted (2026-11-19)
**Date:** 2026-11-19

## Context

The engine is meant to be upgraded for years across multiple games and to be worked on by agents with limited context. Without explicit compatibility tiers, every header becomes a de-facto contract and either change becomes impossible or games break without warning.

## Decision

- **Stable:** `engine/public/` C++ headers and `engine/c_api/`. Follows SemVer. Breaking changes only in a major release, with a changelog entry, migration notes and (for C++) deprecation warnings for at least one minor release.
- **Experimental:** explicitly marked headers/APIs (macro or directory). May change in any minor release; changes must be in the changelog with a migration note.
- **Internal:** everything else under `engine/`. No guarantees; games including internal headers are unsupported and may break in any release.
- **Formats are contracts too:** save files, settings, atlas/animation/tilemap descriptors and packed archives carry versions and follow the same deprecation expectations.
- **C ABI:** append-only structs with a `size` field, opaque handles, an ABI version check at startup; removal requires a major version.
- **Canary:** the example game must build and pass against each release and the previous release, documenting any required migration.
- **Deprecation mechanics:** compile-time warning where possible, changelog "Deprecated" section, documented replacement, listed in the upgrade guide.

## Consequences

- A small stable surface is the goal; adding to `public/` is a deliberate act with long-term cost.
- Some internal convenience must be duplicated rather than promoted; acceptable.
- Releases require an upgrade-guide pass and an example-game migration check, which slows releases slightly.
- Agents get a clear rule for what they may change freely.

## Revisit when

Deprecation windows prove impractical for a single maintainer, or the stable surface becomes so large that maintenance dominates. Adjust the tiers, not the principle.
