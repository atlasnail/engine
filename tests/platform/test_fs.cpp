#include "framework.hpp"

#include <filesystem>
#include <string>
#include <system_error>

#include "eng/platform/fs.hpp"

namespace {

std::filesystem::path test_directory() {
    // CTest runs from the build directory; keep test artifacts inside the project.
    return std::filesystem::current_path() / "eng_fs_test";
}

void reset_test_directory() {
    std::error_code error;
    std::filesystem::remove_all(test_directory(), error);
    std::filesystem::create_directories(test_directory(), error);
}

void cleanup_test_directory() {
    std::error_code error;
    std::filesystem::remove_all(test_directory(), error);
}

}  // namespace

TEST_CASE(fs_write_read_roundtrip) {
    reset_test_directory();
    const std::string directory = test_directory().string();
    const std::string path = eng::fs::join(directory, "hello.txt");

    REQUIRE(eng::fs::write_text_file(path, "hello\nworld").is_ok());

    const eng::Result<std::string> read = eng::fs::read_text_file(path);
    REQUIRE(read.is_ok());
    CHECK(read.value() == "hello\nworld");

    cleanup_test_directory();
}

TEST_CASE(fs_atomic_write_replaces_and_leaves_no_temp) {
    reset_test_directory();
    const std::string directory = test_directory().string();
    const std::string path = eng::fs::join(directory, "save.json");

    REQUIRE(eng::fs::write_text_file(path, "old").is_ok());
    REQUIRE(eng::fs::write_text_file_atomic(path, "new").is_ok());

    const eng::Result<std::string> read = eng::fs::read_text_file(path);
    REQUIRE(read.is_ok());
    CHECK(read.value() == "new");
    CHECK(!eng::fs::exists(path + ".tmp"));

    cleanup_test_directory();
}

TEST_CASE(fs_missing_file_is_not_found) {
    reset_test_directory();
    const std::string path = eng::fs::join(test_directory().string(), "absent.txt");

    const eng::Result<std::string> read = eng::fs::read_text_file(path);
    CHECK(read.is_error());
    CHECK(read.error().code == eng::ErrorCode::not_found);

    cleanup_test_directory();
}

TEST_CASE(fs_creates_directories_and_handles_utf8_names) {
    reset_test_directory();
    const std::string nested = eng::fs::join(test_directory().string(), "caf\u00e9/sub");
    REQUIRE(eng::fs::create_directories(nested).is_ok());
    CHECK(eng::fs::exists(nested));

    const std::string path = eng::fs::join(nested, "na\u00efve.txt");
    REQUIRE(eng::fs::write_text_file(path, "unicode").is_ok());

    const eng::Result<std::string> read = eng::fs::read_text_file(path);
    REQUIRE(read.is_ok());
    CHECK(read.value() == "unicode");

    cleanup_test_directory();
}

TEST_CASE(fs_join_handles_trailing_separators) {
    CHECK(eng::fs::join("", "file.txt") == "file.txt");
    CHECK(eng::fs::join("dir", "file.txt") == "dir/file.txt");
    CHECK(eng::fs::join("dir/", "file.txt") == "dir/file.txt");
}

TEST_CASE(fs_executable_directory_exists) {
    const eng::Result<std::string> directory = eng::fs::executable_directory();
    REQUIRE(directory.is_ok());
    CHECK(!directory.value().empty());
    CHECK(eng::fs::exists(directory.value()));
}

TEST_CASE(fs_create_directories_rejects_existing_file) {
    reset_test_directory();
    const std::string path = eng::fs::join(test_directory().string(), "file.txt");
    REQUIRE(eng::fs::write_text_file(path, "file").is_ok());
    CHECK(eng::fs::create_directories(path).is_error());
    cleanup_test_directory();
}
