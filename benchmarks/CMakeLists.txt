# Benchmark drivers. These are not tests: they run for a chosen duration and
# print measured numbers. See docs/performance.md for methodology.

add_executable(bench_blank bench_blank.cpp)
target_link_libraries(bench_blank PRIVATE eng::platform)
eng_apply_warnings(bench_blank)

if(WIN32)
    target_link_libraries(bench_blank PRIVATE psapi)
endif()

add_custom_target(bench
    COMMAND bench_blank --seconds 5
    DEPENDS bench_blank
    COMMENT "Running bench_blank (5 s, headless)"
    USES_TERMINAL
)
