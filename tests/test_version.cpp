#include "framework.hpp"

#include <string>

#include "peri/core/version.hpp"

TEST_CASE(version_matches_project_version) {
    CHECK(peri::k_engine_version_major == 0);
    CHECK(peri::k_engine_version_minor == 1);
    CHECK(peri::k_engine_version_patch == 0);
    CHECK(std::string(peri::k_engine_version) == "0.1.0");
    CHECK(std::string(PERI_VERSION_STRING) == "0.1.0");
}
