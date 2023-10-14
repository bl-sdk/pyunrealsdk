#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/wrapped_struct.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

WrappedStruct make_struct(const UScriptStruct* type,
                          const py::args& args,
                          const py::kwargs& kwargs) {
    WrappedStruct new_struct{type};

    // Convert the kwarg keys to FNames, to make them case insensitive
    // This should also in theory speed up lookups, since hashing is simpler
    std::unordered_map<FName, py::object> converted_kwargs{};
    std::transform(kwargs.begin(), kwargs.end(),
                   std::inserter(converted_kwargs, converted_kwargs.end()), [](const auto& pair) {
                       return std::make_pair(py::cast<FName>(pair.first),
                                             py::reinterpret_borrow<py::object>(pair.second));
                   });

    size_t arg_idx = 0;
    for (auto prop : type->properties()) {
        if (arg_idx != args.size()) {
            py_setattr(prop, reinterpret_cast<uintptr_t>(new_struct.base.get()), args[arg_idx++]);

            if (converted_kwargs.contains(prop->Name)) {
                throw py::type_error(unrealsdk::fmt::format(
                    "{}.__init__() got multiple values for argument '{}'", type->Name, prop->Name));
            }

            continue;
        }
        // If we're on to just kwargs

        auto iter = converted_kwargs.find(prop->Name);
        if (iter != converted_kwargs.end()) {
            // Use extract to also remove the value from the map, so we can ensure it's empty later
            py_setattr(prop, reinterpret_cast<uintptr_t>(new_struct.base.get()),
                       converted_kwargs.extract(iter).mapped());
            continue;
        }
    }

    if (!converted_kwargs.empty()) {
        // Copying python, we only need to warn about one extra kwarg
        throw py::type_error(
            unrealsdk::fmt::format("{}.__init__() got an unexpected keyword argument '{}'",
                                   type->Name, converted_kwargs.begin()->first));
    }

    return new_struct;
}

void register_wrapped_struct(py::module_& mod) {
    py::class_<WrappedStruct>(
        mod, "WrappedStruct",
        // Need dynamic attr to create a `__dict__`, so that we can handle `__dir__` properly
        py::dynamic_attr())
        .def(py::init(&make_struct),
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
            [](WrappedStruct& self, const FName& name, const py::object& value) {
                py_setattr(py_find_field(name, self.type),
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
                py_setattr(field, reinterpret_cast<uintptr_t>(self.base.get()), value);
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
        .def_readwrite("_type", &WrappedStruct::type);
}

}  // namespace pyunrealsdk::unreal

#endif
