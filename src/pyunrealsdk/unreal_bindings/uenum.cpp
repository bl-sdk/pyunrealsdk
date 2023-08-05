#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/uenum.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/classes/ufield.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

PYUNREALSDK_CAPI(PyObject*, enum_as_py_enum, const UEnum* enum_obj);

#ifdef PYUNREALSDK_INTERNAL

void register_uenum(py::module_& mod) {
    PyUEClass<UEnum, UField>(mod, "UEnum")
        .def("_as_py", &enum_as_py_enum,
             "Generates a compatible IntFlag enum.\n"
             "\n"
             "Returns:\n"
             "    An IntFlag enum compatible with this enum.");
}

py::object enum_as_py_enum(const UEnum* enum_obj) {
    const py::gil_scoped_acquire gil{};

    // Use IntFlag, as it natively supports unknown values
    static auto intflag = py::module_::import("enum").attr("IntFlag");
    static std::unordered_map<const UEnum*, py::object> enum_cache{};

    if (!enum_cache.contains(enum_obj)) {
        enum_cache.emplace(enum_obj, intflag(enum_obj->Name, enum_obj->get_names()));
        enum_cache[enum_obj].attr("_unreal") = enum_obj;
    }

    return enum_cache[enum_obj];
}

PYUNREALSDK_CAPI(PyObject*, enum_as_py_enum, const UEnum* enum_obj) {
    const py::gil_scoped_acquire gil{};

    auto obj = enum_as_py_enum(enum_obj);
    // Add an extra ref, so the python object sticks around when we destroy the py::object
    obj.inc_ref();
    return obj.ptr();
}
#else
py::object enum_as_py_enum(const UEnum* enum_obj) {
    const py::gil_scoped_acquire gil{};

    // Steal the reference which we incremented on the other side of this call
    return py::reinterpret_steal<py::object>(PYUNREALSDK_MANGLE(enum_as_py_enum)(enum_obj));
}
#endif

}  // namespace pyunrealsdk::unreal
