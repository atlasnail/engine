#include "framework.hpp"

#include <utility>

#include "peri/platform/sdl.hpp"

// The platform test binary holds one session for the whole process, so these
// cases can only observe the reference counting, not the final shutdown. That is
// deliberate: tearing SDL down mid-run would break every later test, and the
// shutdown path itself is exercised by every run at exit.

TEST_CASE(sdl_session_is_active_for_the_whole_test_process) {
    CHECK(peri::sdl_session_active());

    {
        const peri::Result<peri::SdlSession> extra = peri::SdlSession::acquire();
        REQUIRE(extra.is_ok());
        CHECK(peri::sdl_session_active());
    }

    // Releasing the extra session must not shut SDL down while the process
    // session is still held.
    CHECK(peri::sdl_session_active());
}

TEST_CASE(sdl_session_move_transfers_the_reference) {
    peri::Result<peri::SdlSession> acquired = peri::SdlSession::acquire();
    REQUIRE(acquired.is_ok());

    peri::SdlSession first = std::move(acquired.value());
    // Moving out of `first` must leave it inert; otherwise the destructor of
    // both objects would release the same reference.
    peri::SdlSession second = std::move(first);

    CHECK(peri::sdl_session_active());
}

TEST_CASE(sdl_session_supports_many_owners) {
    peri::Result<peri::SdlSession> a = peri::SdlSession::acquire();
    peri::Result<peri::SdlSession> b = peri::SdlSession::acquire();
    peri::Result<peri::SdlSession> c = peri::SdlSession::acquire();
    REQUIRE(a.is_ok());
    REQUIRE(b.is_ok());
    REQUIRE(c.is_ok());
    CHECK(peri::sdl_session_active());
}
