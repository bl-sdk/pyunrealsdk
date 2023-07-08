#include "pyunrealsdk/unreal_bindings/wrapped_struct.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_wrapped_struct(py::module_& mod) {
    py::class_<WrappedStruct>(
        mod, "WrappedStruct", "An unreal struct wrapper.",
        // Need dynamic attr to create a `__dict__`, so that we can handle `__dir__` properly
        py::dynamic_attr())
        .def("__new__",
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of wrapped structs.");
             })
        .def("__init__",
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of wrapped structs.");
             })
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

                    auto value = py_getattr(reinterpret_cast<uintptr_t>(self.base.get()), self.type,
                                            py::cast(prop));

                    output << prop->Name << ": " << py::repr(value);
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
            "    A list of attributes which exist on this object.")
        .def(
            "__getattr__",
            [](const WrappedStruct& self, const py::object& key) {
                return py_getattr(reinterpret_cast<uintptr_t>(self.base.get()), self.type, key);
            },
            "Reads an unreal field off of the struct.\n"
            "\n"
            "Usually called with a string holding the field name (as is done in regular\n"
            "attribute access), which automatically looks up the field.\n"
            "\n"
            "In performance critical situations, you can also look up the field beforehand\n"
            "via struct._type._find(\"name\"), then pass the it directly to this function. This\n"
            "does not get validated, passing a field which doesn't exist on the object is\n"
            "undefined behaviour.\n"
            "\n"
            "Note that getattr() only supports string keys, when passing a field you must\n"
            "call this function directly.\n"
            "\n"
            "Args:\n"
            "    key: The field's name, or the field object itself.\n"
            "Returns:\n"
            "    The field's value.",
            "key"_a)
        .def(
            "__setattr__",
            [](WrappedStruct& self, const py::object& key, const py::object& value) {
                py_setattr(reinterpret_cast<uintptr_t>(self.base.get()), self.type, key, value);
            },
            "Writes a value to an unreal property.\n"
            "\n"
            "Usually called with a string holding the field name (as is done in regular\n"
            "attribute access), which automatically looks up the field.\n"
            "\n"
            "In performance critical situations, you can also look up the field beforehand\n"
            "via struct._type._find(\"name\"), then pass the it directly to this function. This\n"
            "does not get validated, passing a field which doesn't exist on the object is\n"
            "undefined behaviour.\n"
            "\n"
            "Note that setattr() only supports string keys, when passing a field you must\n"
            "call this function directly.\n"
            "\n"
            "Args:\n"
            "    key: The field's name, or the field object itself.\n"
            "    value: The value to write.\n"
            "Returns:\n"
            "    The field's value.",
            "key"_a, "value"_a)
        .def_readwrite("_type", &WrappedStruct::type);
}
}  // namespace pyunrealsdk::unreal
