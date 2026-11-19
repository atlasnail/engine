#include "framework.hpp"

#include "leak_check.hpp"

// Entry point for the core test binary. It links only eng::core, so there is no
// SDL session here; the platform binary has its own main.
int main(int argc, char** argv) {
    engtest::CrtLeakCheck leak_check;

    int code = engtest::run_all(argc, argv);

    if (leak_check.report()) {
        code = 1;
    }
    return code;
}
