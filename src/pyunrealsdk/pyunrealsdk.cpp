#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/pyunrealsdk.h"
#include "pyunrealsdk/base_bindings.h"
#include "pyunrealsdk/commands.h"
#include "pyunrealsdk/hooks.h"
#include "pyunrealsdk/logging.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/version.h"
#include "unrealsdk/config.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/version.h"

#ifdef PYUNREALSDK_INTERNAL

// NOLINTNEXTLINE(readability-identifier-length)
PYBIND11_EMBEDDED_MODULE(unrealsdk, m) {
    m.attr("__version__") = unrealsdk::get_version_string();

    // NOLINTBEGIN(readability-magic-numbers)
    auto unrealsdk_version = unrealsdk::get_version();
    m.attr("__version_info__") =
        py::make_tuple((unrealsdk_version >> 16) & 0xFF, (unrealsdk_version >> 8) & 0xFF,
                       (unrealsdk_version >> 0) & 0xFF);
    // NOLINTEND(readability-magic-numbers)

    pyunrealsdk::logging::register_module(m);
    pyunrealsdk::commands::register_module(m);
    pyunrealsdk::unreal::register_module(m);
    pyunrealsdk::hooks::register_module(m);
    pyunrealsdk::register_base_bindings(m);
};

// NOLINTNEXTLINE(readability-identifier-length)
PYBIND11_EMBEDDED_MODULE(pyunrealsdk, m) {
    m.attr("__doc__") =
        "This module exists purely for version information, and has no other contents.";
    m.attr("__version__") = pyunrealsdk::get_version_string();
    m.attr("__version_info__") = py::make_tuple(
        pyunrealsdk::VERSION_MAJOR, pyunrealsdk::VERSION_MINOR, pyunrealsdk::VERSION_PATCH);
};

namespace pyunrealsdk {

namespace {

// We save thread state to release the gil, so our callbacks in other threads can run
// Since this thread gets destroyed, there's little point keeping the thread state around, not like
// we can restore it
// However, Python really doesn't like it if we destroy the startup thread state, I think it tries
// recreating it, it starts throwing all sorts of assertion failures

// It's not a memory leak if we keep a reference around but never touch it again right? :P
PyThreadState* startup_thread_state = nullptr;

}  // namespace

void init(void) {
    while (!unrealsdk::is_initialized()) {}
    LOG(INFO, "{} loaded", pyunrealsdk::get_version_string());

    py::initialize_interpreter(true, 0, nullptr, false);

    // We need to import the sdk module once so that we can use it's types - e.g. we can't cast
    // Logger to a python object when we try overwrite stdout unless it's been imported

    // Need to be careful about destroying it while we still have GIL
    { py::module_::import("unrealsdk"); }

    logging::py_init();

    commands::register_commands();

    try {
        // Use a custom globals to make sure we don't contaminate `py`/`pyexec` commands
        // This also ensures `__file__` gets redefined properly
        py::eval_file(unrealsdk::config::get_str("pyunrealsdk.init_script").value_or("__main__.py"),
                      py::dict{});
    } catch (const std::exception& ex) {
        LOG(ERROR, "Error running python initialization script:");
        logging::log_python_exception(ex);
    }

    startup_thread_state = PyEval_SaveThread();
}

}  // namespace pyunrealsdk

#endif
