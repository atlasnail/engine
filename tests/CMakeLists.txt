add_executable(eng_tests
    main_core.cpp
    test_main.cpp
    test_fixed_step.cpp
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

add_executable(eng_platform_tests
    test_main.cpp
    platform/main_platform.cpp
    platform/test_app.cpp
    platform/test_actions.cpp
    platform/test_file_log.cpp
    platform/test_fs.cpp
    platform/test_sdl.cpp
    platform/test_window.cpp
)

target_link_libraries(eng_platform_tests PRIVATE eng::platform)
target_include_directories(eng_platform_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

eng_apply_warnings(eng_platform_tests)

add_test(NAME eng_platform_tests COMMAND eng_platform_tests)
set_tests_properties(eng_platform_tests PROPERTIES TIMEOUT 60 LABELS "unit;platform")
