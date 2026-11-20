# Benchmark drivers. These are not tests: they run for a chosen duration and
# print measured numbers. See docs/performance.md for methodology.

add_executable(bench_blank bench_blank.cpp)
target_link_libraries(bench_blank PRIVATE peri::platform)
peri_apply_warnings(bench_blank)

if(WIN32)
    target_link_libraries(bench_blank PRIVATE psapi)
endif()

if(PERI_BUILD_TESTS)
    add_test(NAME bench_blank_output
        COMMAND "${CMAKE_COMMAND}"
            "-DBENCH_EXE=$<TARGET_FILE:bench_blank>"
            "-DBENCH_CSV=${CMAKE_CURRENT_BINARY_DIR}/bench-blank-output-test.csv"
            "-DTEST_WINDOWS=${WIN32}"
            -P "${CMAKE_SOURCE_DIR}/tests/test_bench_blank.cmake")
    set_tests_properties(bench_blank_output PROPERTIES
        TIMEOUT 30 LABELS "benchmark;platform"
        ENVIRONMENT "SDL_VIDEODRIVER=dummy")
endif()

add_custom_target(bench
    COMMAND bench_blank --seconds 5
    DEPENDS bench_blank
    COMMENT "Running bench_blank (5 s, headless)"
    USES_TERMINAL
)
