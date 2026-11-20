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
- F1 verdict: **not verified**. At the time of the original run, Linux verification was also outstanding; CI for `92ccfc0` has since passed headless. A validated finer Windows method, Tier A hardware and the full 60-second protocol remain outstanding.

## Windows output safeguard

- On the same `dev-01`, MSVC 19.52 Release, hidden window, overlay and CSV off,
  after disabling Windows benchmark CPU sampling: `build/windows-release/bin/bench_blank.exe --seconds 5 --warmup 1`
  reported 295 measured frames, frame-interval p99 17.216 ms, zero dropped
  steps and 5.5 MB exit private bytes. With `--seconds 3 --warmup 1 --no-limit`,
  it reported 12,982,570 frames, interval p99 0.000 ms in 1 µs buckets, zero
  dropped steps and 5.7 MB. Both printed CPU p99 unavailable. These are single
  short non-gated runs; the changed instrumentation and differing modes prevent
  interpreting them as a controlled performance comparison.
- The focused `ctest --preset windows-release -R bench_blank_output` passed for
  paced and unlimited dummy-driver runs, checking CPU p99 is unavailable and
  the Windows CSV has no CPU column. Linux behavior awaits CI.

## Exploratory cycle probe

- On the same Windows `dev-01`, ran a Python `ctypes` probe against
  `QueryThreadCycleTime(GetCurrentThread())`, 100 iterations per condition.
  Each sample bracketed either no work, `time.sleep(0.01)`, or a Python busy
  loop until `time.perf_counter_ns()` advanced 300,000 ns. Calls, Python work
  and scheduler noise contribute to each sample; these are raw cycle-counter
  deltas, not engine frame measurements and not time in seconds.
- Empty min/median/max: 2,150 / 2,580 / 9,460; sleep: 70,520 / 133,515 /
  209,840; busy: 1,274,520 / 1,293,010 / 1,334,720. No zero samples.
- This shows distinguishable cycles in this probe only. Microsoft explicitly
  warns against converting this counter to elapsed time; no conversion or F1
  conclusion is valid here. `wpr -status` reported not recording. The current
  medium-integrity token was in Performance Log Users, but did not list the
  user profile privilege; no ETW session was started or tested. See
  `docs/performance.md` for the ETW candidate and required validation.

Reproduce from the project root in Git Bash (`python` is the local Windows Python):

```bash
python - <<'PY'
import ctypes, statistics, time
from ctypes import wintypes
k = ctypes.WinDLL('kernel32', use_last_error=True)
k.GetCurrentThread.restype = wintypes.HANDLE
k.QueryThreadCycleTime.argtypes = [wintypes.HANDLE, ctypes.POINTER(ctypes.c_ulonglong)]
k.QueryThreadCycleTime.restype = wintypes.BOOL
thread = k.GetCurrentThread()
def read():
    value = ctypes.c_ulonglong()
    if not k.QueryThreadCycleTime(thread, ctypes.byref(value)):
        raise ctypes.WinError(ctypes.get_last_error())
    return value.value
def trial(kind):
    begin = read()
    if kind == 'busy':
        deadline = time.perf_counter_ns() + 300_000
        while time.perf_counter_ns() < deadline:
            pass
    elif kind == 'sleep':
        time.sleep(.01)
    return read() - begin
for kind in ('empty', 'sleep', 'busy'):
    values = [trial(kind) for _ in range(100)]
    print(kind, 'n=100', 'cycles min/median/max=', min(values),
          statistics.median(values), max(values), 'zeros=', values.count(0))
PY
```
