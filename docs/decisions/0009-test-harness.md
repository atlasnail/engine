# ADR-0009: A minimal in-repository test harness

**Status:** Accepted (2026-11-19)
**Date:** 2026-11-19

## Context

M0 needs a test runner that works identically on Windows and Linux with no
network access, no dependency version drift, and output simple enough for both
humans and agents to read. Candidates: Catch2, doctest, GoogleTest, or a small
in-repo harness.

- Catch2 and GoogleTest are capable but add a dependency, a build step and
  compile time for features the core tests do not use yet.
- doctest is small (single header) but still a third-party file to track and
  update.
- The needs at this stage are narrow: named cases, assertions, a filter, a
  summary and a nonzero exit code on failure.

## Decision

Keep a **~100-line harness in `tests/framework.hpp`** plus `tests/test_main.cpp`.
It provides `TEST_CASE`, `CHECK`, `REQUIRE`, name filtering and a summary. Tests
are registered through a static registrar. No external dependency.

The harness is deliberately limited. If fixtures, parameterized tests,
benchmarking or richer diagnostics become necessary, either extend it with
tests of its own or replace it with a proven framework through a new ADR — do
not grow it into a framework by accident.

## Consequences

- Zero dependency and identical behavior on every platform; agents can read the
  whole harness in one minute.
- Assertions only report file/line/expression. Debugging a failure means
  reproducing the test rather than reading a rich value dump.
- Test discovery is static registration; a test file that is not compiled is
  silently absent, so `tests/CMakeLists.txt` must list every file.
- The final leak check lives in the runner, giving every test suite the same
  memory discipline.

## Revisit when

Fixtures, value-printing assertions, sharding or benchmarking become a real
need, or maintaining the harness costs more than adopting a framework.
