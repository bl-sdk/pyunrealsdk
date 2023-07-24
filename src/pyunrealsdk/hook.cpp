#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/hooks.h"
#include "pyunrealsdk/logging.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/property_proxy.h"

using namespace unrealsdk::unreal;
using namespace unrealsdk::hook_manager;

namespace pyunrealsdk::hooks {

namespace {

// Sentinel class for python.
struct Block {};

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
        cast(hook.ret.prop, [&hook, &ret_arg]<typename T>(const T* /*prop*/) {
            ret_arg = py::cast(hook.ret.get<T>());
        });
    } else {
        ret_arg = py::ellipsis{};
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

            if (py::ellipsis{}.equal(ret_override)) {
                hook.ret.destroy();
            } else {
                cast(hook.ret.prop, [&hook, &ret_override]<typename T>(const T* /*prop*/) {
                    auto value = py::cast<typename PropTraits<T>::Value>(ret_override);
                    hook.ret.set<T>(value);
                });
            }
        }

        should_block = std::move(ret_tuple[0]);
    }

    return py::isinstance<Block>(should_block) || should_block.equal(py::type::of<Block>());
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

    hooks.def("log_all_calls", &log_all_calls,
              "Toggles logging all unreal function calls. Best used in short bursts for\n"
              "debugging.\n"
              "\n"
              "Args:\n"
              "    should_log True to turn on logging all calls, false to turn it off.",
              "should_log"_a);

    hooks.def("inject_next_call", &inject_next_call,
              "Makes the next unreal function call completely ignore hooks.\n"
              "\n"
              "Typically used to avoid recursion when re-calling the hooked function.");

    hooks.def(
        "add_hook",
        [](const std::wstring& func, Type type, const std::wstring& identifier,
           const py::object& callback) {
            add_hook(func, type, identifier, [callback](Details& hook) {
                try {
                    const py::gil_scoped_acquire gil{};
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
        "    ret: The return value of the unreal function. This may have been overwritten\n"
        "         by a previous pre-hook.\n"
        "         Note that while there may be a `ReturnValue` property in the args\n"
        "         struct, it is not necessarily correct, this always will be.\n"
        "    func: The function which was called, bound to the same object. Can be used\n"
        "          to re-call it.\n"
        "\n"
        "Pre-hooks can influence execution of the unreal function: they can block it from\n"
        "running, and/or overwrite it's return value.\n"
        "To block execution, return the special `Block` class (or an instance thereof),\n"
        "either by itself or as the first element of a tuple. Any other value will allow\n"
        "execution to continue - suggest using Ellipsis when a value's required.\n"
        "To overwrite the return value, return it as the second element of a tuple. A\n"
        "value of Ellipsis means \"don't overwrite\". If you need to provide a value,\n"
        "but don't care about what, best practice to forward the value in `ret`.\n"
        "\n"
        "Multiple hooks of the same type on the same function run in an undefined order.\n"
        "Execution is blocked if any hook signals to do so.\n"
        "The return value is overwritten with the value set after calling the final hook.\n"
        "The value passed in `ret` will be set to what the previous hook requested, hooks\n"
        "can try cooperate by inspecting and modifying it.\n"
        "\n"
        "Post-hooks perform the same return value processing, however as the function's\n"
        "already run, the effects are dropped. Overwriting the return value only serves\n"
        "to change what's passed in `ret` during any later hooks.\n"
        "\n"
        "Args\n"
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

}  // namespace pyunrealsdk::hooks
