# Third-party dependencies.
#
# Every dependency is pinned by exact version and integrity hash (ADR-0004).
# Nothing here may be edited in place; update the pin and record why.
#
# SDL3 provides windowing, input, gamepads, timers and (later) audio devices.
# It is built as a static library and used only inside engine/platform, so no
# SDL type appears in engine public headers (ADR-0002).

include(FetchContent)

set(SDL_SHARED OFF CACHE BOOL "Build SDL as a shared library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build SDL as a static library" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build the SDL test library" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Build SDL's own tests" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "Build SDL's examples" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "Install SDL with the project" FORCE)

FetchContent_Declare(SDL3
    URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.20/SDL3-3.4.20.tar.gz
    URL_HASH SHA256=f3bb06f55bed9557227a700af2ea62e49497ebdf58037732a731a5fb3ef8581f
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SYSTEM
)

FetchContent_MakeAvailable(SDL3)
