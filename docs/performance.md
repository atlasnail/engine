# Performance

Performance is a budget, not an aspiration. Every number here is a target with a measurement method, a hardware tier and a milestone gate. Until a benchmark record exists in `benchmarks/`, a target is **unproven**.

## 1. Rules

1. **Baseline before optimization.** Record the current number, then change one thing, then re-measure.
2. **Release builds only** for targets (`RelWithDebInfo` or `Release`, LTO as shipped). Debug numbers are for debugging, not claims.
3. **Report distribution, not averages.** Use median and p99 frame time; an average hides the stutter players feel.
4. **State the machine.** CPU, GPU, RAM, OS build, driver version, display mode, commit hash, build flags.
5. **No target changes without an ADR.** If a target is unreachable, propose a revision with data.
6. **Regressions block.** An unexplained >10 % regression on any gated metric fails the change.
7. **Measure the product, not a microbenchmark**, whenever the product path exists.

## 2. Reference hardware

| Tier | Definition | Use |
| --- | --- | --- |
| **A (primary)** | 4-core/8-thread x86-64 CPU (2018+), integrated GPU, 8 GB RAM, SSD, Windows 11 + Ubuntu LTS | All gated targets |
| **B (floor)** | 2-core/4-thread CPU (2015+), integrated GPU, 4 GB RAM, SATA SSD | Stretch/soak checks; document misses, do not gate 1.0 |
| **Dev** | Whatever the developer uses | Day-to-day only; never cited as evidence |

Record the exact Tier A machine once in `benchmarks/reference-hardware.md` and cite its ID in every result.

## 3. Targets

### Frame time (Tier A, Release, 1080p, VSync off, 60 s after 5 s warm-up)

| ID | Metric | Target | Gate |
| --- | --- | --- | --- |
| F1 | Empty frame CPU time | p99 ≤ 0.3 ms | M1 |
| F2 | 10k sprites, one atlas: CPU submit | p99 ≤ 1.0 ms | M2 |
| F3 | 10k sprites: draw calls | ≤ 100 | M2 |
| F4 | 5k dynamic entities + 10k static colliders: update | p99 ≤ 3.0 ms | M5 |
| F5 | Example game, typical scene: total CPU frame | p99 ≤ 6.0 ms (≈16 % headroom at 60 Hz) | M7 |
| F6 | Frame pacing: frames within ±10 % of target | ≥ 99.5 % over 60 s | M1, M7 |
| F7 | Frame spikes > 2× target | 0 after warm-up, excluding window events/asset loads | M1, M7 |
| F8 | End-to-end input latency | ≤ 2 frames at 60 Hz | M7 |
| F9 | Cold start to first presented frame | empty ≤ 250 ms; example game ≤ 1.5 s | M7 |

Reference budget: at 60 Hz a frame is 16.67 ms. The engine targets **≤ 6 ms CPU** in the example game so game logic, audio and OS jitter have room, and 120 Hz remains viable for simple scenes.

### Memory

| ID | Metric | Target | Gate |
| --- | --- | --- | --- |
| M-1 | Empty app private memory (Windows private working set / Linux RSS) | ≤ 60 MB | M1 |
| M-2 | Example game, steady state, 1080p | ≤ 300 MB total, of which assets ≤ 200 MB | M7 |
| M-3 | Per-frame heap allocations after warm-up | 0 | M2 onward |
| M-4 | Allocation count returns to baseline after scene unload | ±1 % | M5 |
| M-5 | Base entity footprint | ≤ 128 bytes target, ≤ 256 bytes hard ceiling | M5, M7 |
| M-6 | 24 h soak memory growth | ≤ 1 % after warm-up | M7 |
| M-7 | Allocator overhead (metadata + fragmentation) | ≤ 10 % of allocated bytes | M6 |

### Assets, audio, persistence

| ID | Metric | Target | Gate |
| --- | --- | --- | --- |
| A1 | 512 MB PNG load, decode, upload | ≤ 3 s Tier A | M3 |
| A2 | Peak memory during A1 | ≤ 1.6× decoded size (streamed decode) | M3 |
| A3 | Hot reload of one asset | ≤ 100 ms, no frame > 2× target | M3 |
| S1 | Mixer time at 128 voices | p99 ≤ 15 % of buffer duration (≤ 3.2 ms for 1024 frames @ 48 kHz) | M4 |
| S2 | Audio underruns | 0 over 30 min soak | M4 |
| S3 | Output latency at defaults | ≤ 50 ms | M4 |
| P1 | 1 MB save | p99 ≤ 30 ms | M6 |
| P2 | Load 1 MB save | p99 ≤ 20 ms | M6 |
| P3 | Crash handler to dump complete | ≤ 2 s | M6 |
| Q1 | Clean build, engine + example, 8 threads | ≤ 3 min Tier A | M9 |

Targets are ceilings for the engine on Tier A. A game with heavier content can exceed them; the **engine overhead** must not.

## 4. Benchmark suite

Each benchmark is planned as a separate driver under `benchmarks/`. Only `bench_blank` exists so far. It reports elapsed frame-start intervals separately from CPU work when available. On Windows it reports F1 CPU p99 as unavailable and does not sample CPU time; the current clock has not been validated for short frames. Linux has passed headless CI but has not run a performance gate, and the Tier A protocol has not been run.

`bench_blank` accounts for each measured frame using fixed 1 µs histogram buckets
up to 100 ms and an overflow bucket, without retaining individual samples.
Percentiles are the lower bucket edge, or `>=100 ms` for overflow; min, max and
average use the actual samples. Warm-up frames are excluded from the summary.
The optional CSV includes warm-up and has frame interval, update elapsed time and
fixed-step fields. On Linux it also has `cpu_ms`; on Windows that column is
absent, not zero or an estimate. File logging and the overlay affect the
measurement; disable both for the empty-frame CPU baseline.

| Benchmark | What it does | Metrics |
| --- | --- | --- |
| `bench_blank` | Window + loop, nothing drawn | F1, F6, F7, M-1 |
| `bench_sprites` | N sprites (default 10k) from one atlas, camera pan | F2, F3, frame stats |
| `bench_mixed_atlas` | N sprites across M textures | draw calls, batching efficiency |
| `bench_entities` | 5k dynamic entities + 10k static colliders with movement | F4, M-5 |
| `bench_assets` | Load 512 MB of PNGs, then hot reload repeatedly | A1, A2, A3, peak RSS |
| `bench_audio` | 128 voices, music stream, 30 min | S1, S2 |
| `bench_save` | 1 MB and 10 MB save/load cycles, including crash-during-write | P1, P2 |
| `bench_startup` | Cold process start to first frame | F9 |
| `bench_latency` | Synthetic input → visible frame count | F8 |
| `bench_soak` | Example game, scripted play, 24 h | M-2, M-6, S2, crashes |

Rules for adding a benchmark: deterministic content, fixed seed, no network, no human interaction, runs in CI in ≤ 5 min except labeled `soak`.

## 5. Measurement method

| Metric | How |
| --- | --- |
| CPU frame time | On Linux `bench_blank` opts into `CLOCK_THREAD_CPUTIME_ID` around event processing, fixed update, render and optional stats display. It excludes frame limiting, the observer and CSV writes, other threads and GPU work; clock read overhead is included in part. On Windows the benchmark disables CPU measurement and reports F1 p99 unavailable because `GetThreadTimes` is not validated for short frames. Per-phase CPU timers are not implemented. |
| Frame pacing | `bench_blank` currently reports elapsed frame-start intervals, not present timestamps; F6/F7 are not established by its summary. |
| Draw calls / batches | Renderer counters, asserted by the sprites benchmark. |
| Allocations | Tracking allocator: count, bytes, per-subsystem tags; frame-loop counter must be 0 in Release (compile-time-disabled tracking in shipping). |
| Private memory | Windows: `GetProcessMemoryInfo` private working set. Linux: `/proc/self/statm` RSS. Peak via polling at 100 ms; also `heaptrack`/VMMap for investigation. |
| GPU timing | Optional GL timer queries in debug builds; not a gate for 1.0. |
| Audio callback | Monotonic clock around the mix; underrun counter from the device layer. |
| Input latency | Frame counter test: synthetic input at frame N, marker visible at frame N+k; measured over 1000 runs. |
| Load times | Wall clock around the load API; asset count and bytes recorded. |

### Windows CPU-time investigation (2026-11-20)

`GetThreadTimes` reports user and kernel execution time in 100 ns *units*,
but Microsoft does not guarantee 100 ns accounting resolution. On `dev-01`,
the recorded 15.625 ms quantization makes individual short-frame percentiles
unsuitable for F1. A 100-iteration exploratory Python probe observed distinct
`QueryThreadCycleTime` values for empty, sleeping and 300 µs busy sections; the
full conditions and raw summary are in the [developer record](../benchmarks/results/2026-11-20-bench-blank-cpu-win-x64-dev01.md).
This is not a CPU-time validation: Microsoft explicitly warns against
converting those CPU-dependent cycle counts to time. QPC measures elapsed time,
including descheduling; it is not a drop-in CPU-time clock. Calibrating cycles
against QPC alone cannot prove correct behavior across processors, power states,
thread migration or virtual machines.

A candidate for a finer measurement is offline ETW reconstruction: capture
timestamped frame start/end markers tied to the process and main thread, and
intersect each frame interval with that thread's scheduling intervals derived
from `CSwitch` events. Pair transitions correctly across processors, retain
warm-up boundaries, reject missing or ambiguous markers and incomplete traces,
and check all ETW event/buffer loss counters. Validate with repeated short busy
work, sleep, preemption, migration and a longer comparison to `GetThreadTimes`,
and measure tracing overhead before using it for F1. This is a proposal, not an
implemented or validated benchmark. System tracing requires appropriate
privileges/session access; this session did not grant privileges or collect an
ETW trace. Keep the Windows measurement card and F1 gate open.

Sources: [GetThreadTimes](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getthreadtimes),
[QueryThreadCycleTime](https://learn.microsoft.com/en-us/windows/win32/api/realtimeapiset/nf-realtimeapiset-querythreadcycletime),
[QPC and TSC guidance](https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps),
[ETW CSwitch](https://learn.microsoft.com/en-us/windows/win32/etw/cswitch),
[SystemTraceProvider access](https://learn.microsoft.com/en-us/windows/win32/etw/configuring-and-starting-a-systemtraceprovider-session),
and [ETW loss counters](https://learn.microsoft.com/en-us/windows/win32/api/evntrace/ns-evntrace-event_trace_properties).

Method details, exact commands and flags for each benchmark go in the benchmark's README once implemented; `AGENTS.md` §5 must match.

## 6. Recording results

Every gated run writes a record using [`benchmarks/TEMPLATE.md`](../benchmarks/TEMPLATE.md):

- commit hash, branch, build type, compiler version, flags;
- hardware ID and OS/driver versions;
- benchmark name, parameters, duration, warm-up;
- p50/p99/min/max, counters, peak memory;
- raw log/CSV path;
- verdict against target, and any caveat.

Store records as `benchmarks/results/YYYY-MM-DD-<bench>-<platform>.md` plus raw CSV. Keep them in version control; they are the project's memory.

## 7. Regression policy

1. CI runs non-soak benchmarks on Windows and Linux for changes touching `core`, `platform`, `render`, `assets`, `audio`, `scene`.
2. Compare against the latest record for the same hardware tier and build type.
3. >10 % regression or a new frame spike: fix, or justify in the PR with an ADR-style note and update the baseline.
4. Baseline updates require a record, not a number typed into a doc.
5. Monthly (or per milestone): full Tier A run, compare trend, note drift in `docs/status.md`.

## 8. Honest limits

- Integrated GPUs and drivers vary; GL 3.3 behavior can differ across vendors. Targets are CPU-side engine overhead and are measured on the Tier A machine; GPU-bound scenes are the game's responsibility.
- 24 h soak catches slow leaks and device churn, not every race; targeted concurrency tests are separate and planned with the job system.
- Laptop thermal throttling invalidates results: benchmarks record whether the machine was plugged in and thermally settled, and are re-run if numbers look erratic.
