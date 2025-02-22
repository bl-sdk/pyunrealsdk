#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/wrapped_struct.h"
#include "pyunrealsdk/static_py_object.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

namespace {

/**
 * @brief Gets the ignore struct sentinel.
 *
 * @return The ignore struct sentinel.
 */
py::object get_ignore_struct_sentinel(void) {
    PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> storage;
    return storage
        .call_once_and_store_result(
            []() { return py::module_::import("builtins").attr("object")(); })
        .get_stored();
}

}  // namespace

WrappedStruct make_struct(
    std::variant<const unrealsdk::unreal::UFunction*, const unrealsdk::unreal::UScriptStruct*> type,
    const py::args& args,
    const py::kwargs& kwargs) {
    // This function will work for any UStruct, but we deliberately use a variant to only allow
    // scriptstructs and functions, since classes won't necessarily be initialized right
    // Extract the common type back out of it
    const UStruct* struct_type = nullptr;
    std::visit([&struct_type](auto&& val) { struct_type = val; }, type);

    WrappedStruct new_struct{struct_type};
    make_struct(new_struct, args, kwargs);

    return new_struct;
}

void make_struct(unrealsdk::unreal::WrappedStruct& out_struct,
                 const py::args& args,
                 const py::kwargs& kwargs) {
    // Convert the kwarg keys to FNames, to make them case insensitive
    // This should also in theory speed up lookups, since hashing is simpler
    std::unordered_map<FName, py::object> converted_kwargs{};
    std::ranges::transform(
        kwargs, std::inserter(converted_kwargs, converted_kwargs.end()), [](const auto& pair) {
            return std::make_pair(py::cast<FName>(pair.first),
                                  py::reinterpret_borrow<py::object>(pair.second));
        });

    size_t arg_idx = 0;
    for (auto prop : out_struct.type->properties()) {
        if (arg_idx != args.size()) {
            py_setattr_direct(prop, reinterpret_cast<uintptr_t>(out_struct.base.get()),
                              args[arg_idx++]);

            if (converted_kwargs.contains(prop->Name)) {
                throw py::type_error(
                    unrealsdk::fmt::format("{}.__init__() got multiple values for argument '{}'",
                                           out_struct.type->Name, prop->Name));
            }

            continue;
        }
        // If we're on to just kwargs

        auto iter = converted_kwargs.find(prop->Name);
        if (iter != converted_kwargs.end()) {
            // Use extract to also remove the value from the map, so we can ensure it's empty later
            py_setattr_direct(prop, reinterpret_cast<uintptr_t>(out_struct.base.get()),
                              converted_kwargs.extract(iter).mapped());
            continue;
        }
    }

    if (!converted_kwargs.empty()) {
        // Copying python, we only need to warn about one extra kwarg
        throw py::type_error(
            unrealsdk::fmt::format("{}.__init__() got an unexpected keyword argument '{}'",
                                   out_struct.type->Name, converted_kwargs.begin()->first));
    }
}

void register_wrapped_struct(py::module_& mod) {
    py::class_<WrappedStruct>(mod, "WrappedStruct")
        .def(py::init([](std::variant<const unrealsdk::unreal::UFunction*,
                                      const unrealsdk::unreal::UScriptStruct*> type,
                         const py::args& args,
                         const py::kwargs& kwargs) { return make_struct(type, args, kwargs); }),
             "Creates a new wrapped struct.\n"
             "\n"
             "Args:\n"
             "    type: The type of struct to create.\n"
             "    *args: Fields on the struct to initialize.\n"
             "    **kwargs: Fields on the struct to initialize.",
             "type"_a, py::pos_only{})
        .def(
            "__repr__",
            [](const WrappedStruct& self) {
                std::ostringstream output;
                output << "{";

                bool first = true;
                for (const auto& prop : self.type->properties()) {
                    if (!first) {
                        output << ", ";
                    }
                    first = false;
                    output << prop->Name << ": ";

                    try {
                        auto value = py_getattr(prop, reinterpret_cast<uintptr_t>(self.base.get()),
                                                self.base);
                        output << py::repr(value);
                    } catch (...) {
                        output << "<unknown " << prop->Class->Name << ">";
                    }
                }

                output << "}";
                return output.str();
            },
            "Gets a string representation of this struct.\n"
            "\n"
            "Returns:\n"
            "    The string representation.")
        .def(
            "__dir__",
            [](const py::object& self) {
                return py_dir(self, py::cast<WrappedStruct*>(self)->type);
            },
            "Gets the attributes which exist on this struct.\n"
            "\n"
            "Includes both python attributes and unreal fields. This can be changed to only\n"
            "python attributes by calling dir_includes_unreal.\n"
            "\n"
            "Returns:\n"
            "    A list of attributes which exist on this struct.")
        .def(
            "__getattr__",
            [](const WrappedStruct& self, const FName& name) {
                return py_getattr(py_find_field(name, self.type),
                                  reinterpret_cast<uintptr_t>(self.base.get()), self.base);
            },
            "Reads an unreal field off of the struct.\n"
            "\n"
            "Automatically looks up the relevant UField.\n"
            "\n"
            "Args:\n"
            "    name: The name of the field to get.\n"
            "Returns:\n"
            "    The field's value.",
            "name"_a)
        .def(
            "_get_field",
            [](const WrappedStruct& self, UField* field) {
                if (field == nullptr) {
                    throw py::attribute_error("cannot access null attribute");
                }
                return py_getattr(field, reinterpret_cast<uintptr_t>(self.base.get()), self.base);
            },
            "Reads an unreal field off of the struct.\n"
            "\n"
            "In performance critical situations, rather than use __getattr__, you can look up\n"
            "the UField beforehand (via struct._type._find()), then pass it directly to this\n"
            "function. This does not get validated, passing a field which doesn't exist on\n"
            "the struct is undefined behaviour.\n"
            "\n"
            "Args:\n"
            "    field: The field to get.\n"
            "Returns:\n"
            "    The field's value.",
            "field"_a)
        .def(
            "__setattr__",
            [](WrappedStruct& self, const py::str& name, const py::object& value) {
                // See if the standard setattr would work first, in case we're being called on an
                // existing field. Getattr is only called on failure, but setattr is always called.
                PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> storage;
                auto& setattr = storage
                                    .call_once_and_store_result([]() {
                                        return py::module::import("builtins")
                                            .attr("object")
                                            .attr("__setattr__");
                                    })
                                    .get_stored();

                try {
                    setattr(self, name, value);
                    return;
                } catch (py::error_already_set& e) {
                    if (!e.matches(PyExc_AttributeError)) {
                        throw;
                    }
                }

                py_setattr_direct(py_find_field(py::cast<FName>(name), self.type),
                                  reinterpret_cast<uintptr_t>(self.base.get()), value);
            },
            "Writes a value to an unreal field on the struct.\n"
            "\n"
            "Automatically looks up the relevant UField.\n"
            "\n"
            "Args:\n"
            "    name: The name of the field to set.\n"
            "    value: The value to write.",
            "name"_a, "value"_a)
        .def(
            "_set_field",
            [](WrappedStruct& self, UField* field, const py::object& value) {
                if (field == nullptr) {
                    throw py::attribute_error("cannot access null attribute");
                }
                py_setattr_direct(field, reinterpret_cast<uintptr_t>(self.base.get()), value);
            },
            "Writes a value to an unreal field on the struct.\n"
            "\n"
            "In performance critical situations, rather than use __setattr__, you can look up\n"
            "the UField beforehand (via struct._type._find()), then pass it directly to this\n"
            "function. This does not get validated, passing a field which doesn't exist on\n"
            "the struct is undefined behaviour.\n"
            "\n"
            "Args:\n"
            "    field: The field to set.\n"
            "    value: The value to write.",
            "field"_a, "value"_a)
        .def(
            "__copy__", [](const WrappedStruct& self) { return WrappedStruct(self); },
            "Creates a copy of this struct. Don't call this directly, use copy.copy().\n"
            "\n"
            "Returns:\n"
            "    A new, python-owned copy of this struct.")
        .def(
            "__deepcopy__",
            [](const WrappedStruct& self, const py::dict& /*memo*/) { return WrappedStruct(self); },
            "Creates a copy of this struct. Don't call this directly, use copy.deepcopy().\n"
            "\n"
            "Args:\n"
            "    memo: Opaque dict used by deepcopy internals."
            "Returns:\n"
            "    A new, python-owned copy of this struct.",
            "memo"_a)
        .def(
            "_get_address",
            [](const WrappedStruct& self) { return reinterpret_cast<uintptr_t>(self.base.get()); },
            "Gets the address of this struct, for debugging.\n"
            "\n"
            "Returns:\n"
            "    This struct's address.")
        .def_readwrite("_type", &WrappedStruct::type);

    mod.attr("IGNORE_STRUCT") = get_ignore_struct_sentinel();
}

bool is_ignore_struct_sentinel(const py::object& obj) {
    return obj.is(get_ignore_struct_sentinel());
}

}  // namespace pyunrealsdk::unreal

#endif
