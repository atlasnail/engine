#include "framework.hpp"

#include <memory>
#include <string>
#include <utility>

#include "eng/core/result.hpp"

TEST_CASE(result_success_and_failure) {
    using eng::ErrorCode;

    eng::Result<int> good(42);
    CHECK(good.is_ok());
    CHECK(!good.is_error());
    CHECK(static_cast<bool>(good));
    CHECK(good.value() == 42);
    CHECK(good.value_or(7) == 42);

    eng::Result<int> bad(eng::Error{ErrorCode::not_found, "missing"});
    CHECK(bad.is_error());
    CHECK(!bad.is_ok());
    CHECK(!static_cast<bool>(bad));
    CHECK(bad.error().code == ErrorCode::not_found);
    CHECK(std::string(bad.error().message) == "missing");
    CHECK(bad.value_or(7) == 7);
}

TEST_CASE(result_status) {
    using eng::ErrorCode;

    const eng::Status ok = eng::Status::ok();
    CHECK(ok.is_ok());
    CHECK(!ok.is_error());
    CHECK(static_cast<bool>(ok));

    const eng::Status failed = eng::Status::failure(ErrorCode::io_failure, "disk");
    CHECK(failed.is_error());
    CHECK(!static_cast<bool>(failed));
    CHECK(failed.error().code == ErrorCode::io_failure);
}

TEST_CASE(result_move_only_value) {
    eng::Result<std::unique_ptr<int>> result(std::make_unique<int>(11));
    REQUIRE(result.is_ok());
    CHECK(*result.value() == 11);

    eng::Result<std::unique_ptr<int>> moved(std::move(result));
    REQUIRE(moved.is_ok());
    CHECK(*moved.value() == 11);
}

TEST_CASE(result_move_and_copy_string) {
    eng::Result<std::string> text(std::string("hello"));
    REQUIRE(text.is_ok());
    CHECK(text.value() == "hello");

    eng::Result<std::string> copy = text;
    REQUIRE(copy.is_ok());
    CHECK(copy.value() == "hello");

    eng::Result<std::string> moved = std::move(text);
    REQUIRE(moved.is_ok());
    CHECK(moved.value() == "hello");

    copy = moved;
    REQUIRE(copy.is_ok());
    CHECK(copy.value() == "hello");

    eng::Result<std::string> failed(eng::Error{eng::ErrorCode::corrupted_data, "bad"});
    copy = failed;
    CHECK(copy.is_error());
    CHECK(copy.error().code == eng::ErrorCode::corrupted_data);
}

TEST_CASE(result_error_destructor_is_called) {
    struct Tracker {
        int* destructions = nullptr;
        explicit Tracker(int* counter) : destructions(counter) {}
        Tracker(const Tracker&) = delete;
        Tracker& operator=(const Tracker&) = delete;
        Tracker(Tracker&& other) noexcept : destructions(other.destructions) {
            other.destructions = nullptr;
        }
        Tracker& operator=(Tracker&&) = delete;
        ~Tracker() {
            if (destructions != nullptr) {
                ++*destructions;
            }
        }
    };

    int destructions = 0;
    {
        eng::Result<Tracker> result{Tracker(&destructions)};
        CHECK(result.is_ok());
    }
    CHECK(destructions == 1);
}
