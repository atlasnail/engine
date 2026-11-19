# Reference hardware

**Status: placeholder.** Define and record the actual Tier A machine at the start of M1, before the first gated benchmark. Numbers measured on an undefined machine cannot be compared with later results.

## Tier A (primary gate)

| Field | Value |
| --- | --- |
| ID | `tier-a-01` (to assign) |
| CPU | (fill: model, cores/threads, base/boost) |
| GPU | (fill: model, driver version, memory) |
| RAM | (fill: size, speed) |
| Storage | (fill: model, interface) |
| OS | Windows 11 build `<...>`; Ubuntu `<...>` kernel `<...>` |
| Display | resolution, refresh, scaling |
| Audio | device + backend used for latency tests |
| Power/thermal | plugged in, cooling setup |

## Tier B (floor, non-gating)

| Field | Value |
| --- | --- |
| ID | `tier-b-01` (to assign) |
| CPU | 2 cores / 4 threads, 2015+ |
| GPU | integrated |
| RAM | 4 GB |
| Storage | SATA SSD |

## Rules

- Same machine for a target's history; if it changes, re-baseline everything and note the switch.
- Record laptop power mode and thermal state; throttled runs are discarded, not averaged.
- Driver updates are noted in the run record; a driver change requires a re-baseline note.
