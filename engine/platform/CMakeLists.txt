# Platform: window, input, file system and OS boundaries.
#
# SDL is used here and nowhere else. Any OS header belongs in this module.
# Everything above depends on eng::platform through its own types.

add_library(eng_platform STATIC
    src/app.cpp
    src/fs.cpp
    src/file_log.cpp
    src/input.cpp
    src/sdl.cpp
    src/window.cpp
)

add_library(eng::platform ALIAS eng_platform)

target_include_directories(eng_platform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(eng_platform
    PUBLIC eng::core
    PRIVATE SDL3::SDL3-static
)

eng_apply_warnings(eng_platform)
