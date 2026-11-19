# Benchmark record — `bench_blank` on Windows x64 (`dev-01`)

## Environment

| Field | Value |
| --- | --- |
| Commit | `295aebe` (M1 platform layer, recorded 2026-11-19) |
| Branch | `main` |
| Build type | Release, MSVC 19.52, Ninja, warnings-as-errors |
| Hardware ID | `dev-01` (Ryzen 9 9950X3D, RX 9070 XT, 32 GB, Windows 11 26200) |
| Audio | not used by this benchmark |
| Display | headless hidden window, 1280×720 virtual size |
| Power state | plugged in, desktop |

**This is not a Tier A gate run.** `dev-01` is far above the Tier A target, so
these numbers establish a baseline and show the loop behaves correctly; they do
not prove the 1.0 budgets on low-end hardware.

## Run

| Field | Value |
| --- | --- |
| Command (paced) | `bench_blank.exe --seconds 5 --warmup 1 --csv benchmarks/results/2026-11-19-bench_blank-win-x64-dev01.csv` |
| Command (throughput) | `bench_blank.exe --seconds 3 --warmup 1 --no-limit` |
| Repetitions | 1 per mode (first baseline) |
| Date/time | 2026-11-19 |

## Results

### Paced at 60 FPS (SDL_DelayNS limiter, VSync off)

| Metric | Target (Tier A) | Measured (`dev-01`) | Verdict (provisional) |
| --- | --- | --- | --- |
| Frame time p50 | — | 16.988 ms | — |
| Frame time p99 | within ±10 % of 16.67 ms | 17.194 ms (+3.1 %) | pass |
| Frame time min | — | 16.683 ms (+0.1 %) | — |
| Frame time max | within ±10 % | 17.229 ms (+3.4 %) | pass |
| FPS | 60 | 59.0 | expected limiter overshoot |
| Dropped fixed steps | 0 | 0 | pass |
| Private memory at exit | ≤ 60 MB (M-1) | 4.4 MB | pass (provisional) |

### Unlimited throughput (no limiter)

| Metric | Target (Tier A) | Measured (`dev-01`) | Verdict (provisional) |
| --- | --- | --- | --- |
| Empty frame CPU p99 (F1) | ≤ 0.3 ms | < 0.001 ms (rounded to 0.000) | pass (provisional) |
| Empty frame max | — | 0.227 ms | — |
| Dropped fixed steps | 0 | 0 | pass |
| Private memory at exit | ≤ 60 MB | 8.8 MB (includes the 200k-sample vector) | pass (provisional) |

## Artifacts

- Raw CSV (paced run): `benchmarks/results/2026-11-19-bench_blank-win-x64-dev01.csv`
  (357 rows including the 1 s warm-up).

## Notes

- The first baseline; no previous record to compare with.
- Paced mode uses `SDL_DelayNS`; overshoot averages ~0.3 ms, hence 59 FPS rather
  than 60. Every measured frame stayed inside the ±10 % window.
- Unlimited mode is capped at 200,000 samples to keep benchmark memory small;
  p50/p99 are below the 1 µs resolution printed by the summary.
- `F2`–`F9` do not exist yet (no renderer, audio or scene). `F6`/`F7` full
  60-second protocol and Tier A hardware are still pending.
- The benchmark process opens a hidden window; a desktop session is required on
  Windows. On headless Linux CI, set `SDL_VIDEODRIVER=dummy`.

## Baseline decision

- [x] New baseline established (`dev-01`, first record).
- [ ] Compared with previous record.
