#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/hooks.h"
#include "pyunrealsdk/logging.h"
#include "unrealsdk/hook_manager.h"

namespace pyunrealsdk::hooks {

void register_module(py::module_& mod) {
    auto hooks = mod.def_submodule("hooks");

    py::enum_<unrealsdk::hook_manager::Type>(hooks, "Type", "Enum of possible hook types.")
        .value("PRE", unrealsdk::hook_manager::Type::PRE,
               "Called before running the hooked function.")
        .value("POST", unrealsdk::hook_manager::Type::POST,
               "Called after the hooked function, but only if it was allowed to run.")
        .value("POST_UNCONDITIONAL", unrealsdk::hook_manager::Type::POST_UNCONDITIONAL,
               "Called after the hooked function, even if it got blocked.");

    hooks.def(
        "log_all_calls", &unrealsdk::hook_manager::log_all_calls,
        "Toggles logging all unreal function calls. Best used in short bursts for debugging.\n"
        "\n"
        "Args:\n"
        "    should_log True to turn on logging all calls, false to turn it off.",
        "should_log"_a);

    hooks.def("inject_next_call", &unrealsdk::hook_manager::inject_next_call,
              "Makes the next unreal function call completely ignore hooks.\n"
              "\n"
              "Typically used to avoid recursion when re-calling the hooked function.");

    hooks.def(
        "add_hook",
        [](const std::wstring& func, unrealsdk::hook_manager::Type type,
           const std::wstring& identifier, const py::function& callback) {
            unrealsdk::hook_manager::add_hook(
                func, type, identifier, [callback](unrealsdk::hook_manager::Details& hook) {
                    try {
                        py::gil_scoped_acquire gil{};
                        return (bool)py::bool_(callback(hook.obj, hook.args, hook.ret, hook.func));
                    } catch (const std::exception& ex) {
                        logging::log_python_exception(ex);
                    }
                    return false;
                });
        },
        "Adds a hook.\n"
        "\n"
        "Hook callbacks take four positional args. These are, in order:\n"
        "    obj: The object the hooked function was called on.\n"
        "    args: The arguments the hooked function was called with. Note that while\n"
        "          this is mutable, modifying it will *not* modify the actual function\n"
        "          arguments.\n"
        "    ret: A proxy for the return value.\n"
        "         During pre-hooks, it's an unset value (Ellipsis), and setting it will\n"
        "         overwrite the return value of the function call. Whatever value is set\n"
        "         when pre-hook processing is complete will be used, if multiple hooks\n"
        "         all want to overwrite it they will need to cooperate.\n"
        "         During post-hooks, it's set to the return value of the unreal function.\n"
        "         This may have been overwritten by a pre-hook. If execution was blocked\n"
        "         and it was not overwritten during a pre-hook, it will still be unset.\n"
        "         Note that while there may be a `ReturnValue` property in the args\n"
        "         struct, it is not necessarily correct, this always will be.\n"
        "    func: The function which was called, bound to the same object. Can be used\n"
        "          to re-call it.\n"
        "Pre-hooks should return if to block execution - if any pre-hook returns true,\n"
        "the unreal function will not be run.\n"
        "The return value is ignored for the other hook types.\n"
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
