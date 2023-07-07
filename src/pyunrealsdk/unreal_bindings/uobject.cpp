#include "pyunrealsdk/unreal_bindings/uobject.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/format.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unrealsdk.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_uobject(py::module_& mod) {
    PyUEClass<UObject>(
        mod, "UObject", "The base class of all unreal objects.",
        // Need dynamic attr to create a `__dict__`, so that we can handle `__dir__` properly
        py::dynamic_attr())
        .def_no_constructor()
        .def_get_address()
        .def(
            "__str__",
            [](UObject* self) {
                return unrealsdk::fmt::format("{}'{}'", self->Class->Name,
                                              unrealsdk::uobject_path_name(self));
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
            "Includes both python attributes and unreal fields.\n"
            "\n"
            "Returns:\n"
            "    A list of attributes which exist on this object.")
        .def(
            "__getattr__",
            [](UObject* self, const py::object& key) {
                return py_getattr(reinterpret_cast<uintptr_t>(self), self->Class, key, self);
            },
            "Reads an unreal field off of the object.\n"
            "\n"
            "Usually called with a string holding the field name (as is done in regular\n"
            "attribute access), which automatically looks up the field.\n"
            "\n"
            "In performance critical situations, you can also look up the field beforehand\n"
            "via obj.Class.find(\"name\"), then pass the it directly to this function. This does\n"
            "not get validated, passing a field which doesn't exist on the object is\n"
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
            [](UObject* self, const py::object& key, const py::object& value) {
                py_setattr(reinterpret_cast<uintptr_t>(self), self->Class, key, value);
            },
            "Writes a value to an unreal property.\n"
            "\n"
            "Usually called with a string holding the field name (as is done in regular\n"
            "attribute access), which automatically looks up the field.\n"
            "\n"
            "In performance critical situations, you can also look up the field beforehand\n"
            "via obj.Class._find(\"name\"), then pass the it directly to this function. This\n"
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
        .def_readwrite("ObjectFlags", &UObject::ObjectFlags)
        .def_readwrite("InternalIndex", &UObject::InternalIndex)
        .def_readwrite("Class", &UObject::Class)
        .def_readwrite("Name", &UObject::Name)
        .def_readwrite("Outer", &UObject::Outer);
}

}  // namespace pyunrealsdk::unreal
