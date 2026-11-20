// bench_blank: measures the cost of an empty frame (no rendering, no game
// logic). Reports main-thread CPU work separately from elapsed frame intervals.
//
// Usage:
//   bench_blank [--seconds N] [--warmup N] [--visible] [--overlay] [--csv PATH] [--no-limit]
//
// Output is a summary on stdout; the optional CSV has one row per frame.
// Measurement rules live in docs/performance.md.

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

#include "peri/platform/app.hpp"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
// clang-format off
// psapi.h depends on types declared by windows.h, so this order is required.
#include <windows.h>
#include <psapi.h>
// clang-format on
#else
#include <fstream>
#include <string>
#endif

namespace {

// Approximate process memory, reported so the memory gate has a baseline.
// Windows: private bytes. Linux: resident set size from /proc/self/status.
double process_memory_mb() {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS_EX counters{};
    counters.cb = sizeof(counters);
    if (GetProcessMemoryInfo(GetCurrentProcess(),
                             reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters),
                             sizeof(counters)) != 0) {
        return static_cast<double>(counters.PrivateUsage) / (1024.0 * 1024.0);
    }
    return -1.0;
#else
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            return static_cast<double>(std::strtol(line.c_str() + 6, nullptr, 10)) / 1024.0;
        }
    }
    return -1.0;
#endif
}

// 1 us buckets through 100 ms, plus an overflow bucket. Every measured frame
// contributes, with fixed memory and no per-frame allocation.
struct Histogram {
    static constexpr std::size_t k_bins = 100'000;
    std::array<std::uint64_t, k_bins + 1> bins{};
    std::uint64_t count = 0;
    double sum = 0.0;
    double minimum = 0.0;
    double maximum = 0.0;

    void add(double milliseconds) {
        const auto bin =
            milliseconds >= 100.0 ? k_bins : static_cast<std::size_t>(milliseconds * 1000.0);
        ++bins[bin];
        if (count == 0 || milliseconds < minimum) {
            minimum = milliseconds;
        }
        if (count == 0 || milliseconds > maximum) {
            maximum = milliseconds;
        }
        ++count;
        sum += milliseconds;
    }

    [[nodiscard]] std::size_t percentile_bin(double fraction) const {
        const auto rank = static_cast<std::uint64_t>(fraction * static_cast<double>(count - 1));
        std::uint64_t seen = 0;
        for (std::size_t i = 0; i <= k_bins; ++i) {
            seen += bins[i];
            if (seen > rank) {
                return i;
            }
        }
        return k_bins;
    }

    void print(const char* name) const {
        const auto print_percentile = [](std::size_t bin) {
            if (bin == k_bins) {
                std::printf(">=100.000");
            } else {
                std::printf("%.3f", static_cast<double>(bin) / 1000.0);
            }
        };
        std::printf("%s ms: p50 ", name);
        print_percentile(percentile_bin(0.50));
        std::printf("  p99 ");
        print_percentile(percentile_bin(0.99));
        std::printf("  min %.6f  max %.6f  avg %.6f  overflow %llu\n", minimum, maximum,
                    sum / static_cast<double>(count),
                    static_cast<unsigned long long>(bins[k_bins]));
    }
};

}  // namespace

int main(int argc, char** argv) {
    double seconds = 5.0;
    double warmup_seconds = 1.0;
    bool visible = false;
    bool limit = true;
    bool overlay = false;
    const char* csv_path = nullptr;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--seconds") == 0 && i + 1 < argc) {
            seconds = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--warmup") == 0 && i + 1 < argc) {
            warmup_seconds = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--visible") == 0) {
            visible = true;
        } else if (std::strcmp(argv[i], "--csv") == 0 && i + 1 < argc) {
            csv_path = argv[++i];
        } else if (std::strcmp(argv[i], "--no-limit") == 0) {
            limit = false;
        } else if (std::strcmp(argv[i], "--overlay") == 0) {
            overlay = true;
        } else {
            std::printf("usage: %s [--seconds N] [--warmup N] [--visible] [--overlay] [--csv PATH] "
                        "[--no-limit]\n",
                        argv[0]);
            return 2;
        }
    }

    if (!std::isfinite(seconds) || !std::isfinite(warmup_seconds) || seconds <= 0.0 ||
        warmup_seconds < 0.0) {
        std::printf("seconds must be finite and positive; warmup finite and non-negative\n");
        return 2;
    }

    peri::AppConfig config;
    config.window.title = "bench_blank";
    config.window.width = 1280;
    config.window.height = 720;
    config.window.hidden = !visible;
    config.window.vsync = false;
    config.limit_frames = limit;
    config.target_fps = 60.0;
    config.frame_log_path = csv_path;
    config.frame_stats_overlay = overlay;
#if defined(_WIN32)
    // GetThreadTimes has no guaranteed resolution suitable for short frames.
    // Do not emit misleading per-frame CPU samples in either output or CSV.
    config.measure_frame_cpu = false;
#else
    config.measure_frame_cpu = true;
#endif

    peri::Result<std::unique_ptr<peri::App>> app_result = peri::App::create(config);
    if (app_result.is_error()) {
        std::printf("could not create app: %s\n", app_result.error().message);
        return 1;
    }

    std::unique_ptr<peri::App> app = std::move(app_result.value());
    peri::App* app_ptr = app.get();

    auto frame_ms = std::make_unique<Histogram>();
    std::unique_ptr<Histogram> cpu_ms;
    if (config.measure_frame_cpu) {
        cpu_ms = std::make_unique<Histogram>();
    }
    double warmup_elapsed = 0.0;
    double measured_elapsed = 0.0;

    const int exit_code = app->run([](double) {}, [](double) {},
                                   [&frame_ms, &cpu_ms, &warmup_elapsed, &measured_elapsed, app_ptr,
                                    seconds, warmup_seconds](const peri::FrameInfo& info) {
                                       if (warmup_elapsed < warmup_seconds) {
                                           warmup_elapsed += info.frame_seconds;
                                           return;
                                       }
                                       frame_ms->add(info.frame_seconds * 1000.0);
                                       if (cpu_ms) {
                                           cpu_ms->add(info.cpu_seconds * 1000.0);
                                       }
                                       measured_elapsed += info.frame_seconds;
                                       if (measured_elapsed >= seconds) {
                                           app_ptr->quit();
                                       }
                                   });

    if (exit_code != 0) {
        std::printf("benchmark failed; results discarded\n");
        return exit_code;
    }
    if (frame_ms->count == 0) {
        std::printf("no frames recorded\n");
        return 1;
    }

    std::printf("bench_blank: %.2f s measured after %.2f s warmup; %llu frames total, "
                "%llu measured\n",
                measured_elapsed, warmup_seconds,
                static_cast<unsigned long long>(app->frame_count()),
                static_cast<unsigned long long>(frame_ms->count));
    if (limit) {
        std::printf("rate: %.1f fps (target 60)\n",
                    measured_elapsed > 0.0 ? static_cast<double>(frame_ms->count) / measured_elapsed
                                           : 0.0);
    } else {
        std::printf("rate: unlimited\n");
    }
    std::printf("percentiles: 1 us buckets, lower edge; overflow >=100 ms\n");
    frame_ms->print("frame interval");
#if defined(_WIN32)
    std::printf("main-thread CPU: unavailable; GetThreadTimes is not validated for short frames; "
                "F1 CPU p99 unavailable (no CPU samples in CSV)\n");
#else
    cpu_ms->print("main-thread CPU");
    std::printf("CPU clock: CLOCK_THREAD_CPUTIME_ID; main thread only\n");
#endif
    std::printf("dropped fixed steps: %llu\n",
                static_cast<unsigned long long>(app->dropped_steps()));
    std::printf("private memory at exit: %.1f MB\n", process_memory_mb());

    return exit_code;
}
