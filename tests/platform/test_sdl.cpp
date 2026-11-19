#include "framework.hpp"

#include <utility>

#include "eng/platform/sdl.hpp"

// The platform test binary holds one session for the whole process, so these
// cases can only observe the reference counting, not the final shutdown. That is
// deliberate: tearing SDL down mid-run would break every later test, and the
// shutdown path itself is exercised by every run at exit.

TEST_CASE(sdl_session_is_active_for_the_whole_test_process) {
    CHECK(eng::sdl_session_active());

    {
        const eng::Result<eng::SdlSession> extra = eng::SdlSession::acquire();
        REQUIRE(extra.is_ok());
        CHECK(eng::sdl_session_active());
    }

    // Releasing the extra session must not shut SDL down while the process
    // session is still held.
    CHECK(eng::sdl_session_active());
}

TEST_CASE(sdl_session_move_transfers_the_reference) {
    eng::Result<eng::SdlSession> acquired = eng::SdlSession::acquire();
    REQUIRE(acquired.is_ok());

    eng::SdlSession first = std::move(acquired.value());
    // Moving out of `first` must leave it inert; otherwise the destructor of
    // both objects would release the same reference.
    eng::SdlSession second = std::move(first);

    CHECK(eng::sdl_session_active());
}

TEST_CASE(sdl_session_supports_many_owners) {
    eng::Result<eng::SdlSession> a = eng::SdlSession::acquire();
    eng::Result<eng::SdlSession> b = eng::SdlSession::acquire();
    eng::Result<eng::SdlSession> c = eng::SdlSession::acquire();
    REQUIRE(a.is_ok());
    REQUIRE(b.is_ok());
    REQUIRE(c.is_ok());
    CHECK(eng::sdl_session_active());
}
