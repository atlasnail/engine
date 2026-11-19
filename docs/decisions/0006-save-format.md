# ADR-0006: Versioned JSON saves with atomic writes

**Status:** Accepted (2026-11-19)
**Date:** 2026-11-19

## Context

Games need saves that survive crashes, engine upgrades and manual inspection during development. Options: a compact binary format, JSON, or a hybrid (JSON manifest + binary payloads).

Constraints: debuggable and agent-readable, fast enough (1 MB ≤ 30 ms), corruption-detectable, migratable across engine versions, and simple enough that one maintainer can keep it correct. Save sizes for 2D games are small; raw throughput is not the bottleneck.

## Decision

- **JSON (UTF-8)** for save documents, with a small header: format identifier, integer `version`, engine version, timestamp, checksum of the payload.
- **Atomic writes**: write to a temp file in the same directory, flush/fsync where the platform allows, then replace the target; keep the previous save as a backup slot.
- **Checksum** over the payload to detect truncation or tampering.
- **Migrations**: ordered, version-to-version functions, each tested against a stored fixture; unknown/newer versions fail with a clear error, never silently reset.
- Schema documented and validated on load; unknown fields are preserved where practical.
- Binary payloads (rare, e.g. a recorded replay) live in a sidecar file referenced by the JSON, not embedded as base64.

## Consequences

- Saves are inspectable, diffable in development, and easy for agents to work with.
- Larger on disk and slower to parse than binary; acceptable at these sizes, revisited only with data.
- Hand-editable saves require validation to prevent silent corruption.
- The format is a compatibility contract: changes need a version bump and migration.

## Revisit when

Measured save/load latency or file size violates the performance targets or a game needs large binary state (then extend with sidecars, not by replacing the format).
