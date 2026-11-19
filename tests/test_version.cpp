#include "framework.hpp"

#include <string>

#include "eng/core/version.hpp"

TEST_CASE(version_matches_project_version) {
    CHECK(eng::k_engine_version_major == 0);
    CHECK(eng::k_engine_version_minor == 1);
    CHECK(eng::k_engine_version_patch == 0);
    CHECK(std::string(eng::k_engine_version) == "0.1.0");
    CHECK(std::string(ENGINE_VERSION_STRING) == "0.1.0");
}
