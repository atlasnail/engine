# Core: math, errors, logging, memory tracking, time.
# Platform-independent and dependency-free; everything else may depend on it.

add_library(peri_core STATIC
    src/log.cpp
    src/memory.cpp
    src/time.cpp
)

add_library(peri::core ALIAS peri_core)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/peri/core/version.hpp.in"
    "${CMAKE_BINARY_DIR}/generated/peri/core/version.hpp"
    @ONLY
)

target_include_directories(peri_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
        $<INSTALL_INTERFACE:include>
)

peri_apply_warnings(peri_core)
