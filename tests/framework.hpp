#pragma once

// Minimal test harness.
//
// Why not a third-party framework: the engine needs tests that build with zero
// dependencies on every platform and are obvious to read. This provides exactly
// what the unit tests need: named cases, CHECK/REQUIRE, filtering, a summary
// and an exit code. If fixtures or parameterized tests become necessary, add
// them here deliberately (and record an ADR) rather than pulling in a framework.

#include <cstdio>
#include <cstring>
#include <vector>

namespace peritest {

using TestFn = void (*)();

struct TestCase {
    const char* name;
    TestFn fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> cases;
    return cases;
}

inline int& failure_count() {
    static int failures = 0;
    return failures;
}

inline int& check_count() {
    static int checks = 0;
    return checks;
}

inline int& skip_count() {
    static int skips = 0;
    return skips;
}

inline const char*& current_test_name() {
    static const char* name = "";
    return name;
}

struct Registrar {
    Registrar(const char* name, TestFn fn) { registry().push_back(TestCase{name, fn}); }
};

int run_all(int argc, char** argv);

inline void report_failure(const char* file, int line, const char* expression) {
    ++failure_count();
    std::fprintf(stderr, "    FAILED %s:%d: %s\n", file, line, expression);
}

}  // namespace peritest

#define TEST_CASE(identifier)                                                                      \
    static void peritest_case_##identifier();                                                      \
    static ::peritest::Registrar peritest_registrar_##identifier(#identifier,                      \
                                                                 peritest_case_##identifier);      \
    static void peritest_case_##identifier()

#define CHECK(expression)                                                                          \
    do {                                                                                           \
        ++::peritest::check_count();                                                               \
        if (!(expression)) {                                                                       \
            ::peritest::report_failure(__FILE__, __LINE__, #expression);                           \
        }                                                                                          \
    } while (0)

#define REQUIRE(expression)                                                                        \
    do {                                                                                           \
        ++::peritest::check_count();                                                               \
        if (!(expression)) {                                                                       \
            ::peritest::report_failure(__FILE__, __LINE__, #expression);                           \
            return;                                                                                \
        }                                                                                          \
    } while (0)

// Skips the current test. Used where a capability is unavailable (no video
// driver, no GPU) rather than failing, so CI coverage stays honest.
#define SKIP_TEST(reason)                                                                          \
    do {                                                                                           \
        ++::peritest::skip_count();                                                                \
        std::printf("SKIP  %s: %s\n", ::peritest::current_test_name(), reason);                    \
        return;                                                                                    \
    } while (0)
