#include "eng/core/time.hpp"

#include <chrono>

namespace eng::time {

double now_seconds() noexcept {
    // The epoch is initialized on the first call, *before* `now` is read. Do
    // not fold these into one expression: operand evaluation order is
    // unspecified, and reading `now` before the epoch initializes can produce
    // a small negative result (caught by the unit test).
    static const std::chrono::steady_clock::time_point epoch = std::chrono::steady_clock::now();
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = now - epoch;
    return elapsed.count();
}

Stopwatch::Stopwatch() noexcept : started_(now_seconds()) {}

void Stopwatch::reset() noexcept {
    started_ = now_seconds();
}

double Stopwatch::elapsed_seconds() const noexcept {
    return now_seconds() - started_;
}

}  // namespace eng::time
