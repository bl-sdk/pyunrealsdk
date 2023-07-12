#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/hooks.h"
#include "pyunrealsdk/logging.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/cast_prop.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/property_proxy.h"

using namespace unrealsdk::unreal;
using namespace unrealsdk::hook_manager;

namespace pyunrealsdk::hooks {

namespace {

// Sentinel class for python.
struct Block {};

/**
 * @brief Handles calling a pre-hook.
 *
 * @param hook The hook details.
 * @param callback The python hook callback.
 * @return True if to block the function call.
 */
bool handle_prehook(Details& hook, const py::function& callback) {
    auto ret = callback(hook.obj, hook.args /*, hook.func*/);

    if (py::isinstance<py::tuple>(ret)) {
        auto ret_tuple = py::cast<py::tuple>(ret);
        auto size = ret_tuple.size();

        if (size == 1) {
            LOG(WARNING,
                "Return value of pre-hook was tuple of size 1, expected 2. Not overwriting return "
                "value.");
        } else {
            if (size != 2) {
                LOG(WARNING,
                    "Return value of pre-hook was tuple of size {}, expected 2. Extra values will "
                    "be ignored.",
                    size);
            }

            auto ret_override = ret_tuple[1];
            cast_prop(hook.ret.prop, [&hook, &ret_override]<typename T>(const T* /*prop*/) {
                auto value = py::cast<typename PropTraits<T>::Value>(ret_override);
                hook.ret.set<T>(value);
            });
        }

        const py::object execution_state = ret_tuple[0];
        return py::isinstance<Block>(execution_state)
               || execution_state.equal(py::type::of<Block>());
    }

    return py::isinstance<Block>(ret) || ret.equal(py::type::of<Block>());
}

/**
 * @brief Handles calling a post-hook.
 *
 * @param hook The hook details.
 * @param callback The python hook callback.
 */
void handle_posthook(Details& hook, const py::function& callback) {
    py::object ret;
    if (hook.ret.has_value()) {
        cast_prop(hook.ret.prop, [&hook, &ret]<typename T>(const T* /*prop*/) {
            ret = py::cast(hook.ret.get<T>());
        });
    } else {
        ret = py::ellipsis{};
    }

    callback(hook.obj, hook.args /*, hook.func*/, ret);
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
           const py::function& callback) {
            add_hook(func, type, identifier, [type, callback](Details& hook) {
                try {
                    const py::gil_scoped_acquire gil{};

                    if (type == Type::PRE) {
                        return handle_prehook(hook, callback);
                    }

                    handle_posthook(hook, callback);
                    return false;

                } catch (const std::exception& ex) {
                    logging::log_python_exception(ex);
                    return false;
                }
            });
        },
        "Adds a hook.\n"
        "\n"
        "Pre-hook callbacks take three positional args. These are, in order:\n"
        "    obj: The object the hooked function was called on.\n"
        "    args: The arguments the hooked function was called with. Note that while\n"
        "          this is mutable, modifying it will *not* modify the actual function\n"
        "          arguments.\n"
        "    func: The function which was called, bound to the same object. Can be used\n"
        "          to re-call it.\n"
        "Pre-hooks may return the special `Block` value to block execution of the unreal\n"
        "function. Any other value will allow the function to continue.\n"
        "Additionally, if it returns a 2-tuple, the first entry will be used as above,\n"
        "while the second entry will overwrite the function's return value. Suggest using\n"
        "Ellipsis for the first arg if you want to allow the function to continue while\n"
        "overwriting it's return.\n"
        "\n"
        "Post-hook callbacks take a fourth positional arg:\n"
        "    ret: The return value of the unreal function. This may have been overwritten\n"
        "         by a pre-hook. If execution was blocked, and it was not overwritten, it\n"
        "         it will be set to Ellipsis.\n"
        "         Note that while there may be a `ReturnValue` property in the args\n"
        "         struct, it is not necessarily correct, this always will be.\n"
        "Post-hook return values are ignored.\n"
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
              "    func: The function to hook.\n"
              "    type: Which type of hook to check.\n"
              "    identifier: The hook identifier.\n"
              "Returns:\n"
              "    True if a hook with the given details exists.",
              "func"_a, "type"_a, "identifier"_a);

    hooks.def("remove_hook", &unrealsdk::hook_manager::remove_hook,
              "Checks if a hook exists.\n"
              "\n"
              "Args:\n"
              "    func: The function to hook.\n"
              "    type: Which type of hook to check.\n"
              "    identifier: The hook identifier.\n"
              "Returns:\n"
              "    True if a hook with the given details exists.",
              "func"_a, "type"_a, "identifier"_a);
}

}  // namespace pyunrealsdk::hooks
