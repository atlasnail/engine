#pragma once

// Windows CRT heap leak checking for the test executables.
//
// The engine's tracking allocator reports its own allocations; this check is
// different and complementary: it covers anything that reached the CRT heap,
// including third-party code such as SDL and the standard library.
//
// It compares two heap snapshots, taken at the start and end of the run, and
// fails on blocks allocated during the run that were never freed. Comparing
// snapshots rather than dumping everything at exit is deliberate: the harness's
// own registries and the C runtime's stdio buffers are alive by design for the
// whole process, and a whole-heap dump would report them as leaks on every run.
//
// Both calls are no-ops outside Debug builds on Windows, where the CRT debug heap
// does not exist.

#include <cstdio>

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

namespace engtest {

class CrtLeakCheck {
public:
    CrtLeakCheck() {
#if defined(_WIN32) && defined(_DEBUG)
        // ALLOC_MEM_DF records allocations; LEAK_CHECK_DF is deliberately not
        // set because this class reports explicitly and must control the exit
        // code.
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
        _CrtMemCheckpoint(&start_);
#endif
    }

    // Returns true when blocks allocated during the run were still live.
    bool report() {
#if defined(_WIN32) && defined(_DEBUG)
        _CrtMemState end{};
        _CrtMemState difference{};
        _CrtMemCheckpoint(&end);
        if (_CrtMemDifference(&difference, &start_, &end) == 0) {
            return false;
        }
        std::printf("LEAK: the C runtime still holds %lld block(s), %lld byte(s)\n",
                    static_cast<long long>(difference.lCounts[_NORMAL_BLOCK]),
                    static_cast<long long>(difference.lSizes[_NORMAL_BLOCK]));
        _CrtMemDumpStatistics(&difference);
        _CrtMemDumpAllObjectsSince(&start_);
        return true;
#else
        return false;
#endif
    }

private:
#if defined(_WIN32) && defined(_DEBUG)
    _CrtMemState start_{};
#endif
};

}  // namespace engtest
