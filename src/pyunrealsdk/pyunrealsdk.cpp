#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/commands.h"
#include "pyunrealsdk/env.h"
#include "pyunrealsdk/logging.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/version.h"
#include "unrealsdk/unrealsdk.h"

// NOLINTNEXTLINE(readability-identifier-length)
PYBIND11_EMBEDDED_MODULE(unrealsdk, m) {
    pyunrealsdk::logging::register_module(m);
    pyunrealsdk::commands::register_module(m);
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
    // We need to import the sdk module once so that we can use it's types, but we don't need to
    // keep the reference alive.
    { py::module_::import("unrealsdk"); }

    logging::py_init();

    commands::register_commands();

    try {
        py::eval_file(env::get(env::INIT_SCRIPT, env::defaults::INIT_SCRIPT));
    } catch (const std::exception& ex) {
        LOG(ERROR, "Error running python initalization script:");
        logging::log_python_exception(ex);
    }

    startup_thread_state = PyEval_SaveThread();
}

}  // namespace pyunrealsdk
