#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/debugging.h"
#include "pyunrealsdk/env.h"
#include "pyunrealsdk/exports.h"
#include "pyunrealsdk/static_py_object.h"

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

    static StaticPyObject debugpy_debug_this_thread{};

    // Since we initialize with a null object, this is only true on first run - we'll set disabled
    // if we fail to find the object anyway
    if (!(bool)debugpy_debug_this_thread) {
        if (!env::defined(env::DEBUGPY)) {
            disabled = true;
            return;
        }

        {
            const py::gil_scoped_acquire gil{};
            try {
                debugpy_debug_this_thread =
                    py::module_::import("debugpy").attr("debug_this_thread");
            } catch (const py::error_already_set&) {
                disabled = true;
                return;
            }
        }

        if (!(bool)debugpy_debug_this_thread) {
            disabled = true;
            return;
        }
    }

    const py::gil_scoped_acquire gil{};
    debugpy_debug_this_thread();
}

}  // namespace
#endif

}  // namespace pyunrealsdk
