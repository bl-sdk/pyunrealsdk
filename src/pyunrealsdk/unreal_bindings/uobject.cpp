#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/uobject.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/format.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_uobject(py::module_& mod) {
    PyUEClass<UObject>(
        mod, "UObject", "The base class of all unreal objects.",
        // Need dynamic attr to create a `__dict__`, so that we can handle `__dir__` properly
        py::dynamic_attr())
        .def("__new__",
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of unreal objects.");
             })
        .def("__init__",
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of unreal objects.");
             })
        .def(
            "__repr__",
            [](UObject* self) {
                return unrealsdk::fmt::format(
                    "{}'{}'", self->Class->Name,
                    unrealsdk::utils::narrow(unrealsdk::uobject_path_name(self)));
            },
            "Gets this object's name.\n"
            "\n"
            "Returns:\n"
            "    This object's name.")
        .def(
            "__dir__",
            [](const py::object& self) { return py_dir(self, py::cast<UObject*>(self)->Class); },
            "Gets the attributes which exist on this object.\n"
            "\n"
            "Includes both python attributes and unreal fields. This can be changed to only\n"
            "python attributes by calling dir_includes_unreal.\n"
            "\n"
            "Returns:\n"
            "    A list of attributes which exist on this object.")
        .def(
            "__getattr__",
            [](UObject* self, const FName& name) {
                return py_getattr(py_find_field(name, self->Class),
                                  reinterpret_cast<uintptr_t>(self), nullptr, self);
            },
            "Reads an unreal field off of the object.\n"
            "\n"
            "Automatically looks up the relevant UField.\n"
            "\n"
            "Args:\n"
            "    name: The name of the field to get.\n"
            "Returns:\n"
            "    The field's value.",
            "name"_a)
        .def(
            "__getattr__",
            [](UObject* self, UField* field) {
                if (field == nullptr) {
                    throw py::attribute_error("cannot access null attribute");
                }
                return py_getattr(field, reinterpret_cast<uintptr_t>(self), nullptr, self);
            },
            "Reads an unreal field off of the object.\n"
            "\n"
            "In performance critical situations, you can look up the UField beforehand via\n"
            "obj.Class._find(\"name\"), then pass it directly to this function. This does not\n"
            "get validated, passing a field which doesn't exist on the object is undefined\n"
            "behaviour.\n"
            "\n"
            "Note that getattr() only supports string keys, when passing a field you must\n"
            "call this function directly.\n"
            "\n"
            "Args:\n"
            "    field: The field to get.\n"
            "Returns:\n"
            "    The field's value.",
            "field"_a)
        .def(
            "__setattr__",
            [](UObject* self, const FName& name, const py::object& value) {
                py_setattr(py_find_field(name, self->Class), reinterpret_cast<uintptr_t>(self),
                           value);
            },
            "Writes a value to an unreal field on the object.\n"
            "\n"
            "Automatically looks up the relevant UField.\n"
            "\n"
            "Args:\n"
            "    name: The name of the field to set.\n"
            "    value: The value to write.",
            "name"_a, "value"_a)
        .def(
            "__setattr__",
            [](UObject* self, UField* field, const py::object& value) {
                if (field == nullptr) {
                    throw py::attribute_error("cannot access null attribute");
                }
                py_setattr(field, reinterpret_cast<uintptr_t>(self), value);
            },
            "Writes a value to an unreal field on the object.\n"
            "\n"
            "In performance critical situations, you can look up the UField beforehand via\n"
            "obj.Class._find(\"name\"), then pass it directly to this function. This does not\n"
            "get validated, passing a field which doesn't exist on the object is undefined\n"
            "behaviour.\n"
            "\n"
            "Note that setattr() only supports string keys, when passing a field you must\n"
            "call this function directly.\n"
            "\n"
            "Args:\n"
            "    field: The field to set.\n"
            "    value: The value to write.",
            "field"_a, "value"_a)
        .def(
            "_get_address", [](UObject* self) { return reinterpret_cast<uintptr_t>(self); },
            "Gets the address of this object, for debugging.\n"
            "\n",
            "Returns:\n"
            "    This object's address.")
        .def_readwrite("ObjectFlags", &UObject::ObjectFlags)
        .def_readwrite("InternalIndex", &UObject::InternalIndex)
        .def_readwrite("Class", &UObject::Class)
        .def_readwrite("Name", &UObject::Name)
        .def_readwrite("Outer", &UObject::Outer);
}

}  // namespace pyunrealsdk::unreal
