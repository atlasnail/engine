# Benchmark record — CPU instrumentation, Windows `dev-01`

- Date: 2026-11-20; base commit `b2f302f` with uncommitted CPU-instrumentation changes.
- Windows 11 26200, Ryzen 9 9950X3D, RX 9070 XT, 32 GB; Release, MSVC 19.52, Ninja, SDL 3.4.20. Hidden 1280×720 window; overlay off. Developer machine, not Tier A.
- Commands from the project root, one run each:
  - Before: `build/windows-release/bin/bench_blank.exe --seconds 5 --warmup 1`
  - Before: `build/windows-release/bin/bench_blank.exe --seconds 3 --warmup 1 --no-limit`
  - After: `build/windows-release/bin/bench_blank.exe --seconds 5 --warmup 1 --csv build/windows-release/cpu-after.csv`
  - After: `build/windows-release/bin/bench_blank.exe --seconds 3 --warmup 1 --no-limit`
- Before paced: 295 samples, interval p50 16.989 ms, p99 17.253 ms, min 16.686 ms, max 17.527 ms; 4.5 MB exit private bytes. Before unlimited: first 200,000 frames only, 8.4 MB exit private bytes.
- After paced: 294 samples, interval p50 17.038 ms, p99 17.233 ms, min 16.6915 ms, max 17.5339 ms; main-thread CPU p50/p99/min/max 0 ms; 6.5 MB exit private bytes. The raw CSV including warm-up is in the local build directory, not checked in.
- After unlimited: 5,744,481 measured frames across the whole run, interval p50/p99 0.000 ms (1 µs buckets), min 0.0004 ms, max 0.2478 ms; CPU p50/p99 0.000 ms, min 0 ms, max 15.625 ms, average 0.000356 ms; 6.2 MB exit private bytes.
- All runs reported zero dropped fixed steps. The paced commands differ in CSV logging. These single short runs are not a regression assessment and cannot be directly compared as controlled baselines.
- CPU measures main-thread kernel plus user time from `GetThreadTimes`, around event processing through render and stats, excluding pacing, observer and CSV writes. Windows accounting was visibly quantized: a zero percentile does not establish F1. Histograms use 1 µs buckets and include every measured frame; their lower edges are reported. Earlier output used individual frame-start intervals and retained only the first 200,000 frames.
- F1 verdict: **not verified**. A validated finer Windows clock, Linux verification, Tier A hardware and the full 60-second protocol remain outstanding.
