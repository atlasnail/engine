cmake_minimum_required(VERSION 3.25)

project(peridot_engine
    VERSION 0.1.0
    DESCRIPTION "Native 2D game engine for Windows and Linux"
    # C is enabled because pinned dependencies (SDL, the glad loader) are C
    # libraries built inside this project. Engine code itself is C++20.
    LANGUAGES C CXX
)

# ---------------------------------------------------------------------------
# Global settings. These are intentionally strict and identical on every
# platform; per-preset overrides live in CMakePresets.json only.
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

option(PERI_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(PERI_BUILD_TESTS "Build the test suite" ON)
option(PERI_BUILD_BENCHMARKS "Build the benchmark drivers" ON)
option(PERI_SANITIZE "Enable AddressSanitizer and UndefinedBehaviorSanitizer (GCC/Clang)" OFF)

include(cmake/CompilerWarnings.cmake)

if(PERI_SANITIZE)
    if(MSVC)
        message(WARNING "PERI_SANITIZE is not wired up for MSVC yet; ignoring.")
    else()
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

add_subdirectory(third_party)
add_subdirectory(engine/core)
add_subdirectory(engine/platform)

if(PERI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(PERI_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# ---------------------------------------------------------------------------
# Development helper target: `cmake --build build/<preset> --target check`
# configures nothing extra; it runs the tests that were already built.
# ---------------------------------------------------------------------------
if(PERI_BUILD_TESTS)
    add_custom_target(check
        COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Running the test suite with CTest"
        USES_TERMINAL
    )
endif()
