#include "framework.hpp"

#include <chrono>
#include <thread>

#include "eng/core/time.hpp"

TEST_CASE(time_now_seconds_is_monotonic) {
    const double first = eng::time::now_seconds();
    const double second = eng::time::now_seconds();

    CHECK(first >= 0.0);
    CHECK(second >= first);
}

TEST_CASE(time_stopwatch_measures_elapsed) {
    eng::time::Stopwatch watch;
    CHECK(watch.elapsed_seconds() >= 0.0);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const double elapsed = watch.elapsed_seconds();
    CHECK(elapsed >= 0.005);
    CHECK(elapsed < 60.0);

    watch.reset();
    CHECK(watch.elapsed_seconds() < 0.05);
}
