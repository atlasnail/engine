#include "framework.hpp"

#include "peri/core/memory.hpp"

namespace peritest {

int run_all(int argc, char** argv) {
    const char* filter = argc > 1 ? argv[1] : nullptr;

    int ran = 0;
    int skipped = 0;
    int failed_cases = 0;

    for (const TestCase& test : registry()) {
        if (filter != nullptr && std::strstr(test.name, filter) == nullptr) {
            continue;
        }

        const int failures_before = failure_count();
        const int skips_before = skip_count();
        current_test_name() = test.name;

        test.fn();
        ++ran;

        if (skip_count() != skips_before) {
            ++skipped;
        } else if (failure_count() != failures_before) {
            ++failed_cases;
            std::printf("FAIL  %s\n", test.name);
        } else {
            std::printf("ok    %s\n", test.name);
        }
    }

    // A leak anywhere in the engine code is a test failure. The harness's own
    // std::vector storage is not tracked, so this reflects engine allocations
    // only.
    const peri::usize leaked = peri::mem::live_allocations();
    if (leaked != 0) {
        std::printf("LEAK: %zu engine allocation(s) were not released\n", leaked);
        ++failed_cases;
    }

    std::printf("\n%d test case(s) run, %d skipped, %d check(s), %d failed case(s)\n", ran, skipped,
                check_count(), failed_cases);

    if (ran == 0) {
        std::printf("No test cases matched");
        if (filter != nullptr) {
            std::printf(" filter '%s'", filter);
        }
        std::printf(".\n");
        return 1;
    }

    return failed_cases == 0 ? 0 : 1;
}

}  // namespace peritest
