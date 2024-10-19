#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/bound_function.h"
#include "pyunrealsdk/hooks.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/format.h"
#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

namespace {

/**
 * @brief Throws a type error for missing required positional arguments.
 *
 * @param func_name The name of the function which was called.
 * @param missing_required_args The names of the missing required args,
 */
[[noreturn]] void throw_missing_required_args(FName func_name,
                                              const std::vector<FName>& missing_required_args) {
    auto num_missing = missing_required_args.size();

    std::ostringstream stream{};
    stream << func_name << "() missing " << num_missing << " required positional argument";
    if (num_missing > 1) {
        stream << 's';
    }
    stream << ": ";

    for (size_t i = 0; i < num_missing - 1; i++) {
        stream << '\'' << missing_required_args[i] << "', ";
    }
    if (num_missing > 1) {
        stream << "and ";
    }

    stream << '\'' << missing_required_args.back() << '\'';

    throw py::type_error(stream.str());
}

/**
 * @brief Fills the params struct for a function with args from python.
 * @note While this is similar to `make_struct`, we need to do some extra processing on the params,
 *       and we need to fail if an arg was missed.
 *
 * @param info The call info to write to.
 * @param args The python args.
 * @param kwargs The python kwargs.
 */
void fill_py_params(impl::PyCallInfo& info, const py::args& args, const py::kwargs& kwargs) {
    size_t arg_idx = 0;

    std::vector<FName> missing_required_args{};

    for (auto prop : info.params.type->properties()) {
        if ((prop->PropertyFlags & UProperty::PROP_FLAG_PARAM) == 0) {
            continue;
        }
        if ((prop->PropertyFlags & UProperty::PROP_FLAG_RETURN) != 0
            && info.return_param == nullptr) {
            info.return_param = prop;
            continue;
        }
        if ((prop->PropertyFlags & UProperty::PROP_FLAG_OUT) != 0) {
            info.out_params.push_back(prop);
        }

        // If we still have positional args left
        if (arg_idx != args.size()) {
            py_setattr_direct(prop, reinterpret_cast<uintptr_t>(info.params.base.get()),
                              args[arg_idx++]);

            if (kwargs.contains(prop->Name)) {
                throw py::type_error(
                    unrealsdk::fmt::format("{}() got multiple values for argument '{}'",
                                           info.params.type->Name, prop->Name));
            }

            continue;
        }
        // If we're on to just kwargs

        if (kwargs.contains(prop->Name)) {
            // Extract the value with pop, so we can check that kwargs are empty at the
            // end
            py_setattr_direct(prop, reinterpret_cast<uintptr_t>(info.params.base.get()),
                              kwargs.attr("pop")(prop->Name));
            continue;
        }

        // NOLINTNEXTLINE(misc-const-correctness)
        bool optional = false;
#ifdef UE3
        optional = (prop->PropertyFlags & UProperty::PROP_FLAG_OPTIONAL) != 0;
#endif

        // If not given, and not optional, record for error later
        if (!optional) {
            missing_required_args.push_back(prop->Name);
        }
    }

    if (!missing_required_args.empty()) {
        throw_missing_required_args(info.params.type->Name, missing_required_args);
    }

    if (!kwargs.empty()) {
        // Copying python, we only need to warn about one extra kwarg
        std::string bad_kwarg = py::str(kwargs.begin()->first);
        throw py::type_error(unrealsdk::fmt::format("{}() got an unexpected keyword argument '{}'",
                                                    info.params.type->Name, bad_kwarg));
    }
}

}  // namespace

namespace impl {

PyCallInfo::PyCallInfo(const UFunction* func, const py::args& args, const py::kwargs& kwargs)
    // Start by initializing a null struct, to avoid allocations
    : params(func, nullptr) {
    if (func->NumParams < args.size()) {
        throw py::type_error(
            unrealsdk::fmt::format("{}() takes {} positional args, but {} were given", func->Name,
                                   func->NumParams, args.size()));
    }

    // If we're given exactly one arg, and it's a wrapped struct of our function type, take it as
    // the args directly
    if (args.size() == 1 && kwargs.empty() && py::isinstance<WrappedStruct>(args[0])) {
        auto args_struct = py::cast<WrappedStruct>(args[0]);
        if (args_struct.type == func) {
            this->params = std::move(args_struct);

            // Manually gather the return value and out params
            for (auto prop : func->properties()) {
                if ((prop->PropertyFlags & UProperty::PROP_FLAG_RETURN) != 0
                    && return_param == nullptr) {
                    this->return_param = prop;
                    continue;
                }
                if ((prop->PropertyFlags & UProperty::PROP_FLAG_OUT) != 0) {
                    this->out_params.push_back(prop);
                }
            }
            return;
        }
    }

    // Otherwise, allocate a new params struct
    this->params = WrappedStruct{func};
    fill_py_params(*this, args, kwargs);
}

py::object PyCallInfo::get_py_return(void) const {
    const py::list ret{1 + this->out_params.size()};

    if (this->return_param == nullptr) {
        ret[0] = py::ellipsis{};
    } else {
        ret[0] =
            py_getattr(this->return_param, reinterpret_cast<uintptr_t>(this->params.base.get()),
                       this->params.base);
    }

    auto idx = 1;
    for (auto prop : this->out_params) {
        ret[idx++] = py_getattr(prop, reinterpret_cast<uintptr_t>(this->params.base.get()),
                                this->params.base);
    }

    if (this->out_params.empty()) {
        return ret[0];
    }
    return py::tuple(ret);
}

}  // namespace impl

void register_bound_function(py::module_& mod) {
    py::class_<BoundFunction>(mod, "BoundFunction")
        .def(py::init<UFunction*, UObject*>(),
             "Creates a new bound function.\n"
             "\n"
             "Args:\n"
             "    func: The function to bind.\n"
             "    object: The object the function is bound to.",
             "func"_a, "object"_a)
        .def(
            "__repr__",
            [](BoundFunction& self) {
                return unrealsdk::fmt::format(
                    "<bound function {} on {}>", self.func->Name,
                    unrealsdk::utils::narrow(self.object->get_path_name()));
            },
            "Gets a string representation of this function and the object it's bound to.\n"
            "\n"
            "Returns:\n"
            "    The string representation.")
        .def(
            "__call__",
            [](BoundFunction& self, const py::args& args, const py::kwargs& kwargs) {
                impl::PyCallInfo info{self.func, args, kwargs};

                if (hooks::should_auto_inject_py_calls()) {
                    unrealsdk::hook_manager::inject_next_call();
                }

                // Release the GIL to avoid a deadlock if ProcessEvent is locking.
                // If a hook tries to call into Python, it will be holding the process event lock,
                // and it will try to acquire the GIL.
                // If at the same time python code on a different thread tries to call an unreal
                // function, it'd be holding the GIL, and trying to acquire the process event lock.
                {
                    const py::gil_scoped_release gil{};
                    self.call<void>(info.params);
                }

                return info.get_py_return();
            },
            "Calls the function.\n"
            "\n"
            "Args:\n"
            "    The unreal function's args. Out params will be used to initialized the\n"
            "    unreal value, but the python value is not modified in place. Kwargs are\n"
            "    supported.\n"
#ifdef UE3
            "    Optional params should also be optional.\n"
#endif
            "    Alternatively, may call with a single positional WrappedStruct which matches\n"
            "    the type of the function, in order to reuse the args already stored in it.\n"
            "Returns:\n"
            "    If the function has no out params, returns the actual return value, or\n"
            "    Ellipsis for a void function.\n"
            "    If there are out params, returns a tuple, where the first entry is the\n"
            "    return value as described above, and the following entries are the final\n"
            "    values of each of the out params, in positional order.")
        .def_readwrite("func", &BoundFunction::func)
        .def_readwrite("object", &BoundFunction::object);
}

}  // namespace pyunrealsdk::unreal

#endif
