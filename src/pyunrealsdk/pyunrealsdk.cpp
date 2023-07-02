#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/logging.h"
#include "pyunrealsdk/version.h"
#include "unrealsdk/unrealsdk.h"

// NOLINTNEXTLINE(readability-identifier-length)
PYBIND11_EMBEDDED_MODULE(unrealsdk, m) {
    pyunrealsdk::logging::register_module(m);
};

namespace pyunrealsdk {

void init(void) {
    while (!unrealsdk::is_initialized()) {}
    LOG(INFO, "{} loaded", pyunrealsdk::get_version_string());

    py::initialize_interpreter(true, 0, nullptr, false);
    py::module_::import("unrealsdk");

    logging::py_init();

    try {
        py::exec(R"(
        import unrealsdk.logging as log
        import sys

        print(sys.stdout.level)
        sys.stdout.level = log.Level.MISC
        log.dev_warning("test")
        print(sys.stdout.level)
        sys.stderr.write("stderr test\n")
    )");
    } catch (const std::exception& ex) {
        LOG(ERROR, "{}", ex.what());
    }
}

}  // namespace pyunrealsdk
