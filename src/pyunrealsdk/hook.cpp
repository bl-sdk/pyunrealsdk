#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/debugging.h"
#include "pyunrealsdk/exports.h"
#include "pyunrealsdk/hooks.h"
#include "pyunrealsdk/logging.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/property_proxy.h"

using namespace unrealsdk::unreal;
using namespace unrealsdk::hook_manager;

namespace pyunrealsdk::hooks {

PYUNREALSDK_CAPI(bool, is_block_sentinel, PyObject* obj);

#ifdef PYUNREALSDK_INTERNAL

namespace {

// Sentinel classes for python.
struct Block {};
struct Unset {};

// And we need an empty dummy class to create the context manager on.
struct AutoInjectContextManager {};

thread_local size_t auto_inject_count = 0;

}  // namespace

bool should_auto_inject_py_calls(void) {
    return auto_inject_count > 0;
}

namespace {

/**
 * @brief Handles calling a python hook.
 *
 * @param hook The hook details.
 * @param callback The python hook callback.
 * @return True if to block the function call.
 */
bool handle_py_hook(Details& hook, const py::object& callback) {
    py::object ret_arg;
    if (hook.ret.has_value()) {
        cast(hook.ret.prop, [&hook, &ret_arg]<typename T>(T* prop) {
            ret_arg = pyunrealsdk::unreal::py_getattr(
                prop, reinterpret_cast<uintptr_t>(hook.ret.ptr.get()), hook.ret.ptr);
        });
    } else {
        ret_arg = py::type::of<Unset>();
    }

    auto should_block = callback(hook.obj, hook.args, ret_arg, hook.func);

    if (py::isinstance<py::tuple>(should_block)) {
        auto ret_tuple = py::cast<py::tuple>(should_block);
        auto size = ret_tuple.size();

        if (size == 1) {
            LOG(WARNING,
                "Return value of hook was tuple of size 1, expected 2. Not overwriting return "
                "value.");
        } else {
            if (size != 2) {
                LOG(WARNING,
                    "Return value of hook was tuple of size {}, expected 2. Extra values will be "
                    "ignored.",
                    size);
            }

            auto ret_override = ret_tuple[1];

            if (py::type::of<Unset>().is(ret_override) || py::isinstance<Unset>(ret_override)) {
                hook.ret.destroy();
            } else if (!py::ellipsis{}.equal(ret_override)) {
                cast(hook.ret.prop, [&hook, &ret_override]<typename T>(T* prop) {
                    // Need to replicate PropertyProxy::set ourselves a bit, since we want to use
                    // our custom python setter
                    if (hook.ret.ptr.get() == nullptr) {
                        hook.ret.ptr = UnrealPointer<void>(hook.ret.prop);
                    }

                    pyunrealsdk::unreal::py_setattr_direct(
                        prop, reinterpret_cast<uintptr_t>(hook.ret.ptr.get()), ret_override);
                });
            }
        }

        should_block = std::move(ret_tuple[0]);
    }

    return is_block_sentinel(should_block);
}

}  // namespace

void register_module(py::module_& mod) {
    auto hooks = mod.def_submodule("hooks");

    py::enum_<Type>(hooks, "Type", "Enum of possible hook types.")
        .value("PRE", Type::PRE, "Called before running the hooked function.")
        .value("POST", Type::POST,
               "Called after the hooked function, but only if it was allowed to run.")
        .value("POST_UNCONDITIONAL", Type::POST_UNCONDITIONAL,
               "Called after the hooked function, even if it got blocked.");

    py::class_<Block>(
        hooks, "Block",
        "A sentinel used to indicate a hook should block execution of the unrealscript\n"
        "function.")
        .def(py::init<>());

    py::class_<Unset>(
        hooks, "Unset",
        "A sentinel used to indicate a return value override is unset - i.e. the actual\n"
        "return value will be used.")
        .def(py::init<>());

    // Create under an empty handle to prevent this type being normally accessible
    py::class_<AutoInjectContextManager>(py::handle(), "context_manager", pybind11::module_local())
        .def("__enter__", [](const py::object& /*self*/) { auto_inject_count++; })
        .def("__exit__", [](const py::object& /*self */, const py::object& /*exc_type*/,
                            const py::object& /*exc_value*/, const py::object& /*traceback*/) {
            if (auto_inject_count > 0) {
                auto_inject_count--;
            }
        });

    hooks.def("log_all_calls", &log_all_calls,
              "Toggles logging all unreal function calls. Best used in short bursts for\n"
              "debugging.\n"
              "\n"
              "Args:\n"
              "    should_log: True to turn on logging all calls, false to turn it off.",
              "should_log"_a);

    hooks.def(
        "inject_next_call",
        []() {
            if (PyErr_WarnEx(PyExc_DeprecationWarning,
                             "inject_next_call is deprecated, use the"
                             " prevent_hooking_direct_calls() context manager instead.",
                             1)
                == -1) {
                throw pybind11::error_already_set();
            }
            inject_next_call();
        },
        "Makes the next unreal function call completely ignore hooks.\n"
        "\n"
        "Typically used to avoid recursion when re-calling the hooked function.\n"
        "\n"
        "Deprecated in favour of the prevent_hooking_direct_calls() context manager.");

    hooks.def(
        "prevent_hooking_direct_calls", []() { return AutoInjectContextManager{}; },
        "Context manager to prevent direct calls to unreal functions triggering hooks.\n"
        "\n"
        "Typically used to avoid recursion when re-calling the hooked function.\n"
        "\n"
        "Note this only affects direct calls to BoundFunction.__call__(). If the unreal\n"
        "function itself calls other functions, those will still trigger hooks as normal.\n"
        "\n"
        "Returns:\n"
        "    A new context manager.");

    hooks.def(
        "add_hook",
        [](const std::wstring& func, Type type, const std::wstring& identifier,
           const py::object& callback) {
            add_hook(func, type, identifier, [callback](Details& hook) {
                try {
                    const py::gil_scoped_acquire gil{};
                    debug_this_thread();

                    return handle_py_hook(hook, callback);

                } catch (const std::exception& ex) {
                    logging::log_python_exception(ex);
                    return false;
                }
            });
        },
        "Adds a hook which runs when an unreal function is called.\n"
        "\n"
        "Hook callbacks take four positional args. These are, in order:\n"
        "    obj: The object the hooked function was called on.\n"
        "    args: The arguments the hooked function was called with. Note that while\n"
        "          this is mutable, modifying it will *not* modify the actual function\n"
        "          arguments.\n"
        "    ret: The return value of the unreal function, or the value of the previous\n"
        "         return value override, if it has yet to run.\n"
        "         Note that while there may be a `ReturnValue` property in the args\n"
        "         struct, it is not necessarily correct, this always will be.\n"
        "    func: The function which was called, bound to the same object. Can be used\n"
        "          to re-call it.\n"
        "\n"
        "Pre-hooks can influence execution of the unreal function: they can block it from\n"
        "running, and/or overwrite it's return value.\n"
        "\n"
        "To block execution, return the sentinel `Block` type, (or an instance thereof),\n"
        "either by itself or as the first element of a tuple. Any other value will allow\n"
        "execution continue - though generally just use None. If there are multiple hooks\n"
        "on the same function, execution is blocked if any hook requests it.\n"
        "\n"
        "To overwrite the return value, return it as the second element of a tuple. The\n"
        "the sentinel `Unset` type will prevent an override, while using Ellipsis will\n"
        "forward the previous value (rather than needing to copy it from `ret`). If there\n"
        "are multiple hooks on the same function, they will be run in an undefined order,\n"
        "where each hook is passed the previous override's value in `ret`, and the value\n"
        "returned by the final hook is what will be used.\n"
        "\n"
        "Post-hooks perform the same block/return override value processing, however as\n"
        "the function's already run, the effects are dropped. Overwriting the return\n"
        "value only serves to change what's passed in `ret` during any later hooks.\n"
        "\n"
        "Args:\n"
        "    func: The function to hook.\n"
        "    type: Which type of hook to add.\n"
        "    identifier: The hook identifier.\n"
        "    callback: The callback to run when the hooked function is called.\n"
        "Returns:\n"
        "    True if successfully added, false if an identical hook already existed.",
        "func"_a, "type"_a, "identifier"_a, "callback"_a);

    hooks.def("has_hook", &unrealsdk::hook_manager::has_hook,
              "Checks if a hook exists.\n"
              "\n"
              "Args:\n"
              "    func: The function to check.\n"
              "    type: The type of hook to check.\n"
              "    identifier: The hook identifier.\n"
              "Returns:\n"
              "    True if a hook with the given details exists.",
              "func"_a, "type"_a, "identifier"_a);

    hooks.def("remove_hook", &unrealsdk::hook_manager::remove_hook,
              "Removes an existing hook.\n"
              "\n"
              "Args:\n"
              "    func: The function to remove hooks from.\n"
              "    type: The type of hook to remove.\n"
              "    identifier: The hook identifier.\n"
              "Returns:\n"
              "    True if successfully removed, false if no hook with the given details exists.",
              "func"_a, "type"_a, "identifier"_a);
}

PYUNREALSDK_CAPI(bool, is_block_sentinel, PyObject* obj) {
    const py::gil_scoped_acquire gil{};

    // Borrow, so that we add our own reference to it
    auto py_obj = py::reinterpret_borrow<py::object>(obj);
    return py::isinstance<Block>(py_obj) || py::type::of<Block>().is(py_obj);
}

#endif

bool is_block_sentinel(const py::object& obj) {
    return PYUNREALSDK_MANGLE(is_block_sentinel)(obj.ptr());
}

}  // namespace pyunrealsdk::hooks
