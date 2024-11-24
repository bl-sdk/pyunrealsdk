#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/debugging.h"
#include "pyunrealsdk/exports.h"
#include "pyunrealsdk/static_py_object.h"
#include "unrealsdk/config.h"

namespace pyunrealsdk {

namespace {

PYUNREALSDK_CAPI(void, debug_this_thread);

}

void debug_this_thread(void) {
    PYUNREALSDK_MANGLE(debug_this_thread)();
}

#ifdef PYUNREALSDK_INTERNAL
namespace {

PYUNREALSDK_CAPI(void, debug_this_thread) {
    // We want the disabled case to be as early an exit as possible
    static bool disabled = false;
    if (disabled) {
        return;
    }

    const py::gil_scoped_acquire gil{};

    PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> storage;
    auto& debugpy_debug_this_thread =
        storage
            .call_once_and_store_result([]() -> py::object {
                if (unrealsdk::config::get_bool("pyunrealsdk.debugpy").value_or(false)) {
                    try {
                        return py::module_::import("debugpy").attr("debug_this_thread");
                    } catch (const py::error_already_set&) {}
                }
                disabled = true;
                return py::none{};
            })
            .get_stored();

    if (disabled) {
        return;
    }

    debugpy_debug_this_thread();
}

}  // namespace
#endif

}  // namespace pyunrealsdk
