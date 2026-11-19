add_executable(eng_tests
    test_main.cpp
    test_log.cpp
    test_math.cpp
    test_memory.cpp
    test_result.cpp
    test_time.cpp
    test_version.cpp
)

target_link_libraries(eng_tests PRIVATE eng::core)
target_include_directories(eng_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

eng_apply_warnings(eng_tests)

add_test(NAME eng_tests COMMAND eng_tests)
set_tests_properties(eng_tests PROPERTIES TIMEOUT 60 LABELS "unit")
