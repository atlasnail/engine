// bench_blank: measures the cost of an empty frame (no rendering, no game
// logic). This is the M1 gate F1/F6/F7 and the baseline every later feature is
// compared against.
//
// Usage:
//   bench_blank [--seconds N] [--warmup N] [--visible] [--csv PATH] [--no-limit]
//
// Output is a summary on stdout; the optional CSV has one row per frame.
// Measurement rules live in docs/performance.md.

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

#include "eng/platform/app.hpp"

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

double percentile_sorted(const std::vector<double>& sorted, double fraction) {
    if (sorted.empty()) {
        return 0.0;
    }
    const double index = fraction * static_cast<double>(sorted.size() - 1);
    return sorted[static_cast<std::size_t>(index)];
}

}  // namespace

int main(int argc, char** argv) {
    double seconds = 5.0;
    double warmup_seconds = 1.0;
    bool visible = false;
    bool limit = true;
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
        } else {
            std::printf(
                "usage: %s [--seconds N] [--warmup N] [--visible] [--csv PATH] [--no-limit]\n",
                argv[0]);
            return 2;
        }
    }

    if (seconds <= 0.0 || warmup_seconds < 0.0) {
        std::printf("seconds must be positive and warmup non-negative\n");
        return 2;
    }

    eng::AppConfig config;
    config.window.title = "bench_blank";
    config.window.width = 1280;
    config.window.height = 720;
    config.window.hidden = !visible;
    config.window.vsync = false;
    config.limit_frames = limit;
    config.target_fps = 60.0;
    config.frame_log_path = csv_path;

    eng::Result<std::unique_ptr<eng::App>> app_result = eng::App::create(config);
    if (app_result.is_error()) {
        std::printf("could not create app: %s\n", app_result.error().message);
        return 1;
    }

    std::unique_ptr<eng::App> app = std::move(app_result.value());
    eng::App* app_ptr = app.get();

    // Sample cap keeps an unlimited-throughput run from using tens of MB of
    // benchmark memory; p99 over 200k frames is stable.
    constexpr std::size_t k_sample_cap = 200'000;
    std::vector<double> frame_ms;
    frame_ms.reserve(
        std::min<std::size_t>(k_sample_cap, static_cast<std::size_t>(seconds * 240.0) + 1024));
    double warmup_elapsed = 0.0;
    double measured_elapsed = 0.0;

    const int exit_code = app->run([](double) {}, [](double) {},
                                   [&frame_ms, &warmup_elapsed, &measured_elapsed, app_ptr, seconds,
                                    warmup_seconds](const eng::FrameInfo& info) {
                                       if (warmup_elapsed < warmup_seconds) {
                                           warmup_elapsed += info.frame_seconds;
                                           return;
                                       }
                                       if (frame_ms.size() < k_sample_cap) {
                                           frame_ms.push_back(info.frame_seconds * 1000.0);
                                       }
                                       measured_elapsed += info.frame_seconds;
                                       if (measured_elapsed >= seconds) {
                                           app_ptr->quit();
                                       }
                                   });

    if (frame_ms.empty()) {
        std::printf("no frames recorded\n");
        return 1;
    }

    std::vector<double> sorted = frame_ms;
    std::sort(sorted.begin(), sorted.end());

    double total_ms = 0.0;
    for (const double value : frame_ms) {
        total_ms += value;
    }
    const double average_ms = total_ms / static_cast<double>(frame_ms.size());

    std::printf("bench_blank: %.2f s measured after %.2f s warmup; %llu frames total, "
                "%zu sampled\n",
                measured_elapsed, warmup_seconds,
                static_cast<unsigned long long>(app->frame_count()), frame_ms.size());
    if (limit) {
        std::printf("rate: %.1f fps (target 60)\n",
                    measured_elapsed > 0.0 ? static_cast<double>(frame_ms.size()) / measured_elapsed
                                           : 0.0);
    } else {
        std::printf("rate: unlimited (measurement samples capped at %zu)\n", frame_ms.size());
    }
    std::printf("frame ms: p50 %.3f  p99 %.3f  min %.3f  max %.3f  avg %.3f\n",
                percentile_sorted(sorted, 0.50), percentile_sorted(sorted, 0.99), sorted.front(),
                sorted.back(), average_ms);
    std::printf("dropped fixed steps: %llu\n",
                static_cast<unsigned long long>(app->dropped_steps()));
    std::printf("private memory at exit: %.1f MB\n", process_memory_mb());

    return exit_code;
}
