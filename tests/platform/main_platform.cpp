#include <cstdio>

#include "framework.hpp"
#include "leak_check.hpp"
#include "peri/platform/sdl.hpp"

// Entry point for the platform test binary.
//
// The SDL session is scoped so it is released, and SDL_Quit has run, before the
// leak check reports: otherwise SDL's own live state would be reported as a leak
// and the check would be meaningless.
int main(int argc, char** argv) {
    peritest::CrtLeakCheck leak_check;

    int code = 0;
    {
        const peri::Result<peri::SdlSession> session = peri::SdlSession::acquire();
        if (session.is_error()) {
            std::fprintf(stderr, "could not initialise SDL: %s\n", session.error().message);
            return 1;
        }
        code = peritest::run_all(argc, argv);
    }

    if (leak_check.report()) {
        code = 1;
    }
    return code;
}
