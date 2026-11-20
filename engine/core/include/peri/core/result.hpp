#pragma once

// Error handling.
//
// The engine never throws across a module or public API boundary. A fallible
// function returns either:
//   - `Result<T>`: a value on success, an `Error` on failure.
//   - `Status`:    no value, just success or an `Error`.
//
// Error messages are non-owning `const char*` (normally a string literal).
// This keeps errors allocation-free and cheap to pass across layers. If you
// need rich context (a file path, a size), log it at the point of detection
// and keep the returned message short and stable.

#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

#include "peri/core/types.hpp"

namespace peri {

enum class ErrorCode : u8 {
    none = 0,
    invalid_argument,
    not_found,
    io_failure,
    out_of_memory,
    unsupported,
    corrupted_data,
    internal_error,
};

struct Error {
    ErrorCode code = ErrorCode::none;
    const char* message = "";

    constexpr Error() = default;
    constexpr Error(ErrorCode code_in, const char* message_in)
        : code(code_in), message(message_in) {}

    constexpr explicit operator bool() const { return code != ErrorCode::none; }
};

// Success-or-error with no value.
class [[nodiscard]] Status {
public:
    constexpr Status() = default;
    constexpr Status(Error error) : error_(error) {}

    static constexpr Status ok() { return Status{}; }
    static constexpr Status failure(ErrorCode code, const char* message) {
        return Status(Error{code, message});
    }

    [[nodiscard]] constexpr bool is_ok() const { return error_.code == ErrorCode::none; }
    [[nodiscard]] constexpr bool is_error() const { return !is_ok(); }
    [[nodiscard]] constexpr explicit operator bool() const { return is_ok(); }

    // Only valid when is_error() is true.
    [[nodiscard]] constexpr Error error() const {
        assert(is_error());
        return error_;
    }

private:
    Error error_{};
};

// A value on success, an Error on failure.
//
// Storage is a manually managed union so that `T` does not need to be default
// constructible and no sentinel value is required. Copy operations exist but
// only compile when `T` is copyable; move-only values work.
template <typename T>
class [[nodiscard]] Result {
    static_assert(!std::is_reference_v<T>, "Result<T&> is not supported; use a pointer");
    static_assert(!std::is_same_v<T, Error>, "Use Status instead of Result<Error>");
    static_assert(std::is_move_constructible_v<T>, "Result<T> requires a move-constructible T");

public:
    using value_type = T;

    Result(T value) noexcept(std::is_nothrow_move_constructible_v<T>) : ok_(true) {
        ::new (static_cast<void*>(&storage_)) T(std::move(value));
    }

    Result(Error error) noexcept : ok_(false) {
        ::new (static_cast<void*>(&storage_)) Error(error);
    }

    Result(const Result& other) : ok_(other.ok_) {
        if (ok_) {
            ::new (static_cast<void*>(&storage_)) T(other.storage_.value_);
        } else {
            ::new (static_cast<void*>(&storage_)) Error(other.storage_.error_);
        }
    }

    Result(Result&& other) noexcept(std::is_nothrow_move_constructible_v<T>) : ok_(other.ok_) {
        if (ok_) {
            ::new (static_cast<void*>(&storage_)) T(std::move(other.storage_.value_));
        } else {
            ::new (static_cast<void*>(&storage_)) Error(other.storage_.error_);
        }
    }

    Result& operator=(const Result& other) {
        if (this != &other) {
            destroy();
            ok_ = other.ok_;
            if (ok_) {
                ::new (static_cast<void*>(&storage_)) T(other.storage_.value_);
            } else {
                ::new (static_cast<void*>(&storage_)) Error(other.storage_.error_);
            }
        }
        return *this;
    }

    Result& operator=(Result&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
        if (this != &other) {
            destroy();
            ok_ = other.ok_;
            if (ok_) {
                ::new (static_cast<void*>(&storage_)) T(std::move(other.storage_.value_));
            } else {
                ::new (static_cast<void*>(&storage_)) Error(other.storage_.error_);
            }
        }
        return *this;
    }

    ~Result() { destroy(); }

    [[nodiscard]] bool is_ok() const { return ok_; }
    [[nodiscard]] bool is_error() const { return !ok_; }
    [[nodiscard]] explicit operator bool() const { return ok_; }

    // Precondition: is_ok(). Checked with an assertion in debug builds.
    T& value() & {
        assert(ok_);
        return storage_.value_;
    }
    const T& value() const& {
        assert(ok_);
        return storage_.value_;
    }
    T&& value() && {
        assert(ok_);
        return static_cast<T&&>(storage_.value_);
    }

    // Precondition: is_error().
    [[nodiscard]] Error error() const {
        assert(!ok_);
        return storage_.error_;
    }

    // Returns the value on success, or `fallback` on failure.
    template <typename U>
    [[nodiscard]] T value_or(U&& fallback) const& {
        return ok_ ? storage_.value_ : static_cast<T>(std::forward<U>(fallback));
    }

private:
    union Storage {
        T value_;
        Error error_;

        Storage() {}
        ~Storage() {}
    };

    void destroy() noexcept {
        if (ok_) {
            storage_.value_.~T();
        } else {
            storage_.error_.~Error();
        }
    }

    bool ok_;
    Storage storage_;
};

template <typename T>
Result(T) -> Result<T>;

}  // namespace peri
