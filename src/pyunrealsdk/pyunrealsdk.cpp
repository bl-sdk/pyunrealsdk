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
        import unrealsdk

        # unrealsdk.set_console_log_level(unrealsdk.LogLevel.DEV_WARNING)

        print(unrealsdk.LogLevel.__doc__)
        unrealsdk.log("test", level=unrealsdk.LogLevel.WARNING)
    )");
    } catch (const std::exception& ex) {
        LOG(ERROR, "{}", ex.what());
    }
}

}  // namespace pyunrealsdk
