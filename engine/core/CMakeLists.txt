# Core: math, errors, logging, memory tracking, time.
# Platform-independent and dependency-free; everything else may depend on it.

add_library(eng_core STATIC
    src/log.cpp
    src/memory.cpp
    src/time.cpp
)

add_library(eng::core ALIAS eng_core)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/eng/core/version.hpp.in"
    "${CMAKE_BINARY_DIR}/generated/eng/core/version.hpp"
    @ONLY
)

target_include_directories(eng_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
        $<INSTALL_INTERFACE:include>
)

eng_apply_warnings(eng_core)
