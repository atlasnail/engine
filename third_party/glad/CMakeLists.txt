# Vendored glad output: GL 3.3 core, no extensions (see README.md here).
#
# This is third-party generated code. It is deliberately not given the project's
# warning set: the code is not ours to edit, and turning on stricter warnings
# would mean patching generated files. Includes are marked SYSTEM so its headers
# do not produce diagnostics in engine code either.

add_library(peri_gl_loader STATIC src/gl.c)

add_library(peri::gl_loader ALIAS peri_gl_loader)

target_include_directories(peri_gl_loader SYSTEM
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Feeds the later install/export step so games still find glad/gl.h.
        $<INSTALL_INTERFACE:include>
)

# Keep the generated C out of the project's C++ warning policy, but still build
# it with a normal warning level instead of none.
if(MSVC)
    target_compile_options(peri_gl_loader PRIVATE /W3)
else()
    target_compile_options(peri_gl_loader PRIVATE -Wall)
endif()
