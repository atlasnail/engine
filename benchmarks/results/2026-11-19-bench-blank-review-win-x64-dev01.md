# Benchmark record — M1 platform review (`dev-01`)

- Date: 2026-11-19; base commit `4b692c8` plus uncommitted M1 review changes.
- Windows 11 26200, Ryzen 9 9950X3D, RX 9070 XT, 32 GB; desktop, plugged in.
- Release, MSVC 19.52, Ninja, SDL 3.4.20; hidden 1280×720 window; overlay off.
- Commands (one run each, from project root):
  - `build/windows-release/bin/bench_blank.exe --seconds 5 --warmup 1 --csv build/windows-release/bench-review.csv`
  - `build/windows-release/bin/bench_blank.exe --seconds 3 --warmup 1 --no-limit`
- Paced: 295 sampled frames, p50 17.022 ms, p99 17.491 ms, min 16.681 ms,
  max 17.652 ms, average 16.983 ms, 58.9 FPS, zero dropped steps, 4.5 MB
  private bytes at exit. Raw CSV: `2026-11-19-bench-blank-review-win-x64-dev01.csv`
  (includes warm-up).
- Unlimited: 200,000 samples from the first part of the 3-second run, p50 and
  p99 rounded to 0.000 ms, max 0.156 ms, zero dropped steps, 8.2 MB at exit.
- Previous `dev-01` paced p99 was 17.194 ms and exit private bytes 4.4 MB.
  This short, single run is noisy and does not demonstrate a performance change.
- These are wall-clock frame-start intervals, **not CPU time**, and unlimited
  sampling is capped. F1 is not proven. This is not Tier A and not the 60-second
  gate protocol. Overlay, file logging and configured actions were disabled.
