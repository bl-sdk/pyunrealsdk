#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/env.h"
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
        py::eval_file(env::get(env::INIT_SCRIPT, env::defaults::INIT_SCRIPT));
    } catch (const std::exception& ex) {
        LOG(ERROR, "Error running python initalization script:");
        LOG(ERROR, "{}", ex.what());
    }
}

}  // namespace pyunrealsdk
