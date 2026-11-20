# ADR-0011: Peridot Engine name and brand

**Status:** Accepted for project use; name and mark not legally cleared (2026-11-20)
**Date:** 2026-11-20

## Context

The owner chose Peridot Engine and the shorthand `peri` in place of the
placeholder `engine` / `eng`. Consistent C++ identifiers and build targets avoid
carrying the placeholder into future APIs. Existing uses of Peridot in the game
industry mean that adoption is not a substitute for a trademark search.

## Decision

- Use **Peridot Engine** in product-facing prose and `peri` for C++ namespaces,
  include prefixes, CMake targets, test binaries and `PERI_` macros/options.
  CMake project name: `peridot_engine`. Keep the existing `engine/` directory,
  benchmark records and actual repository URL; they identify real paths and
  historical evidence.
- Make a single breaking change now while the implemented headers are internal
  under ADR-0007. No stable `engine/public/` or `engine/c_api/` interface has
  been implemented. Old namespace, include and target aliases are not maintained.
  Record migration details in the changelog and include them in the next 0.x
  minor release notes. Existing save/asset formats are not changed.
- Use an original geometric, faceted gem as the SVG mark, with a horizontal
  outlined wordmark and monochrome variants. Keep usage guidance in `brand/`.
- Vendor **Manrope variable 4.505**, from Google Fonts commit
  `b31870aff700ab7a1d74fa0c6887d95beb9e0037`, SHA-256
  `3ae11c49db0455a3cc33e37d380f20fdb8c7f8b41dc07625c177e3d87a9d6ae6`.
  Its SIL OFL 1.1 allows commercial use. Include its copyright and license with
  the font; it is a brand asset, not an engine runtime dependency. Upstream is
  maintained via Google Fonts; updates must be reviewed and pinned. See
  `third_party/README.md` and `brand/fonts/OFL.txt`.

## Consequences

Downstream users of the old internal interfaces must update their includes,
namespace qualifiers, target names, build options and macros. Old CMake cache
values do not automatically migrate: configure with `--fresh` and reapply
custom options using the new names; use a new build directory if stale outputs
would be confusing. The existing repository and historical outputs keep their
previous paths and names. See `docs/status.md` for build verification. The new name and logo require jurisdiction-specific
trademark and common-law review before public release, and may need to change.

## Revisit when

Trademark review identifies a conflict, the stable API is introduced, or the
brand assets or font need an explicit update.
