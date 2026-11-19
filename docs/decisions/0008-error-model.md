# ADR-0008: Error values at boundaries, no exceptions in the public API

**Status:** Accepted (2026-11-19)
**Date:** 2026-11-19

## Context

The engine must handle missing files, corrupt assets, lost audio devices and script errors without crashing the game or terminating the process. C++ exceptions complicate binary boundaries, encourage hidden control flow in hot paths, interact poorly with callbacks and the C ABI, and make behavior harder to reason about for agents reading unfamiliar code. Error codes alone can be ignored silently and lose context.

Options: exceptions throughout; error codes plus logging; `Result<T>`-style values; abort on error.

## Decision

- **No exceptions cross module boundaries or the public API.** The C ABI is exception-free by construction.
- Fallible public functions return **error values** (`Result<T>`, status enums) that carry a category and message; callers must handle or explicitly propagate them.
- Failures are logged once at the boundary where they are detected, with path/context, then returned; avoid double logging.
- `assert` is for programmer errors in Debug only. Recoverable conditions never assert in Release.
- Unrecoverable startup failures (no window, no GL context) log, attempt a crash dump, and exit with a documented nonzero code.
- Third-party code that throws (e.g. `std::bad_alloc`) is caught at module boundaries and converted.
- Exceptions are permitted in tools and tests where they simplify code.

## Consequences

- Predictable control flow and clean C interop; easier reasoning for agents and reviewers.
- Callers must check results; API design and tests must make ignoring them awkward (nodiscard, test coverage).
- Some boilerplate for propagation; a small helper set keeps it terse.
- Exception-based libraries need wrappers at the boundary.

## Revisit when

Result propagation demonstrably causes bugs or excessive complexity across the codebase, or a required dependency cannot be wrapped.
