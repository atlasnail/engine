#pragma once

// Monotonic time.
//
// The engine never uses wall-clock time for frame timing; wall clocks jump when
// the system time is adjusted, monotonic clocks do not. `now_seconds()` is
// seconds since an unspecified, process-local epoch and is only meaningful for
// differences.

namespace eng::time {

[[nodiscard]] double now_seconds() noexcept;

// Simple elapsed-time helper. Copyable and cheap; not thread-safe by itself,
// but each instance is independent.
class Stopwatch {
public:
    Stopwatch() noexcept;

    void reset() noexcept;
    [[nodiscard]] double elapsed_seconds() const noexcept;

private:
    double started_ = 0.0;
};

}  // namespace eng::time
