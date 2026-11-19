# Reference hardware

**Status: partially defined.** The Tier A machine (the gating hardware) still has
to be designated. Until it is, numbers from `dev-01` are baselines, not gate
results.

## Tier A (primary gate)

| Field | Value |
| --- | --- |
| ID | `tier-a-01` (to assign) |
| CPU | (fill: 4-core/8-thread, 2018 or newer) |
| GPU | (fill: integrated) |
| RAM | (fill: 8 GB) |
| Storage | (fill: SSD) |
| OS | Windows 11; Ubuntu LTS |
| Display | resolution, refresh, scaling |
| Audio | device + backend used for latency tests |
| Power/thermal | plugged in, cooling setup |

## Dev machine (baseline only, not a gate)

| Field | Value |
| --- | --- |
| ID | `dev-01` |
| CPU | AMD Ryzen 9 9950X3D, 16 cores / 32 threads |
| GPU | AMD Radeon RX 9070 XT, driver 32.0.32015.16001 |
| RAM | 32 GB |
| OS | Windows 11 Pro, build 10.0.26200 |
| Compiler | MSVC 19.52.37029, Clang 22.1.3 |
| Display | (not recorded for headless benchmark) |
| Notes | Far above Tier A; use for development and relative changes only. |

## Tier B (floor, non-gating)

| Field | Value |
| --- | --- |
| ID | `tier-b-01` (to assign) |
| CPU | 2 cores / 4 threads, 2015+ |
| GPU | integrated |
| RAM | 4 GB |
| Storage | SATA SSD |

## Rules

- Same machine for a target's history; if it changes, re-baseline everything and
  note the switch.
- Record laptop power mode and thermal state; throttled runs are discarded, not
  averaged.
- Driver updates are noted in the run record; a driver change requires a
  re-baseline note.
