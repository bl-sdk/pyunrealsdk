#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/uenum.h"
#include "pyunrealsdk/static_py_object.h"
#include "pyunrealsdk/stubgen.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/classes/ufield.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

PYUNREALSDK_CAPI(PyObject*, enum_as_py_enum, const UEnum* enum_obj);

#ifdef PYUNREALSDK_INTERNAL

void register_uenum(py::module_& mod) {
    PYUNREALSDK_STUBGEN_MODULE_N("unrealsdk.unreal")

    PyUEClass<UEnum, UField>(mod, PYUNREALSDK_STUBGEN_CLASS("UEnum", "UField"))
        .def(PYUNREALSDK_STUBGEN_METHOD("_as_py", "type[_GenericUnrealEnum]"), &enum_as_py_enum,
             PYUNREALSDK_STUBGEN_DOCSTRING("Generates a compatible IntFlag enum.\n"
                                           "\n"
                                           "Returns:\n"
                                           "    An IntFlag enum compatible with this enum.\n"));
}

py::object enum_as_py_enum(const UEnum* enum_obj) {
    const py::gil_scoped_acquire gil{};

    // Use IntFlag, as it natively supports unknown values
    PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> storage;
    auto& intflag = storage
                        .call_once_and_store_result(
                            []() { return py::module_::import("enum").attr("IntFlag"); })
                        .get_stored();

    static std::unordered_map<const UEnum*, StaticPyObject> enum_cache{};

    if (!enum_cache.contains(enum_obj)) {
        auto py_enum = intflag(enum_obj->Name(), enum_obj->get_names());
        py_enum.attr("_unreal") = enum_obj;

        enum_cache.emplace(enum_obj, py_enum);
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
