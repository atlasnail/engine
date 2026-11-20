#pragma once

// Assertions for programmer errors only.
//
// Recoverable conditions (a missing file, a corrupt save, a lost audio device)
// must use Result/Status instead. An assertion means "this can never happen if
// the code is correct", and it does nothing in release builds.
//
// PERI_ASSERT logs a fatal record, then breaks into the debugger when one is
// attached. It intentionally does not abort in release because the condition
// compiles out entirely.

#include "peri/core/log.hpp"

#if defined(_MSC_VER)
#define PERI_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define PERI_DEBUG_BREAK() __builtin_trap()
#else
#include <cstdlib>
#define PERI_DEBUG_BREAK() std::abort()
#endif

#if defined(NDEBUG)
#define PERI_ASSERT(condition) ((void)0)
#else
#define PERI_ASSERT(condition)                                                                     \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            ::peri::log_message(::peri::LogLevel::fatal, "assert", "assertion failed: %s (%s:%d)", \
                                #condition, __FILE__, static_cast<int>(__LINE__));                 \
            PERI_DEBUG_BREAK();                                                                    \
        }                                                                                          \
    } while (0)
#endif
