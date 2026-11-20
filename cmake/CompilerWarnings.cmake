# Warning policy shared by every target.
#
# Warnings are errors by default (PERI_WARNINGS_AS_ERRORS). The goal is that a
# clean build is the normal state, so a new warning is always a real signal.

function(peri_apply_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4
            /permissive-
            /utf-8
            /Zc:__cplusplus
            /EHsc
        )
        if(PERI_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wcast-align
            -Wnull-dereference
            -Woverloaded-virtual
            -Wdouble-promotion
            -Wconversion
            -Wsign-conversion
        )
        # Conversion warnings are noisy while APIs are young. Keep them
        # visible, but do not fail the build on them yet. Revisit at M1.
        target_compile_options(${target} PRIVATE -Wno-error=conversion -Wno-error=sign-conversion)
        if(PERI_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()
