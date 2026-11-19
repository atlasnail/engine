# Benchmark record — `<benchmark>` on `<platform>`

Copy this file to `benchmarks/results/YYYY-MM-DD-<bench>-<platform>.md` for every gated run. Keep raw logs/CSVs next to it and commit both.

## Environment

| Field | Value |
| --- | --- |
| Commit | `<hash>` (clean working tree: yes/no) |
| Branch | |
| Build type | Release / RelWithDebInfo (state flags, LTO) |
| Compiler | MSVC `<version>` / GCC `<version>` / Clang `<version>` |
| Hardware ID | e.g. `tier-a-01` (see `benchmarks/reference-hardware.md`) |
| CPU / GPU / RAM | |
| OS build / kernel | |
| GPU driver version | |
| Audio device/backend | |
| Display / resolution / refresh | |
| Power state | plugged in / battery, thermal notes |

## Run

| Field | Value |
| --- | --- |
| Command | exact invocation |
| Parameters | e.g. sprites=10000, atlas=mixed, duration=60s |
| Warm-up | |
| Repetitions | 3, median reported |
| Date/time | |

## Results

| Metric | Target | p50 | p99 | min | max | Verdict |
| --- | --- | --- | --- | --- | --- | --- |
| | | | | | | pass/fail |

Counters (draw calls, batches, allocations, peak memory, underruns, etc.):

## Artifacts

- Raw log: `benchmarks/results/<file>.log`
- CSV: `benchmarks/results/<file>.csv`
- Screenshot/trace (if any):

## Notes

Caveats, anomalies, thermal throttling, known noise sources, comparison to the previous record.

## Baseline decision

- [ ] New baseline established (reason):
- [ ] Compared with previous record (`<path>`): delta `<%>`; explained by:
