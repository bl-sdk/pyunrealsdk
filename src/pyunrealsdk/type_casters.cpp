#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/type_casters.h"
#include "pyunrealsdk/exports.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::type_casters {

#ifdef PYUNREALSDK_INTERNAL

const void* downcast_unreal(const UObject* src, const std::type_info*& type) {
    if (src != nullptr) {
        cast<cast_options<true, true>>(
            src,
            // On successful cast: return the templated type
            [&type]<typename T>(const T* /*obj*/) { type = &typeid(T); },
            // On error: fall back to UObject
            [&type](const UObject* /*obj*/) { type = &typeid(UObject); });
    }

    return src;
}

#endif

PYUNREALSDK_CAPI(PyObject*, cast_from_object, UObject* src);
PYUNREALSDK_CAPI(PyObject*, cast_from_struct, WrappedStruct* src);

#ifdef PYUNREALSDK_INTERNAL

// Do a pybind cast, then increment the reference counter to keep the python object alive as we
// return, after the `py::object` gets destroyed

PYUNREALSDK_CAPI(PyObject*, cast_from_object, UObject* src) {
    const py::gil_scoped_acquire gil{};
    auto obj = py::cast(src);
    obj.inc_ref();
    return obj.ptr();
}
PYUNREALSDK_CAPI(PyObject*, cast_from_struct, WrappedStruct* src) {
    const py::gil_scoped_acquire gil{};
    auto obj = py::cast(src);
    obj.inc_ref();
    return obj.ptr();
}

#else

// Steal the reference which we incremented on the other side of the call

py::object cast(UObject* src) {
    const py::gil_scoped_acquire gil{};
    return py::reinterpret_steal<py::object>(PYUNREALSDK_MANGLE(cast_from_object)(src));
}
py::object cast(WrappedStruct* src) {
    const py::gil_scoped_acquire gil{};
    return py::reinterpret_steal<py::object>(PYUNREALSDK_MANGLE(cast_from_struct)(src));
}

#endif

PYUNREALSDK_CAPI(UObject*, cast_to_object, PyObject* src);

#ifdef PYUNREALSDK_INTERNAL

// Borrow the reference which was passed into this function, so that we add our own + remove it on
// returning

PYUNREALSDK_CAPI(UObject*, cast_to_object, PyObject* src) {
    const py::gil_scoped_acquire gil{};
    return py::cast<UObject*>(py::reinterpret_borrow<py::object>(src));
}

#else

template <>
UObject* cast<UObject*>(const py::object& src) {
    return PYUNREALSDK_MANGLE(cast_to_object)(src.ptr());
}

#endif

}  // namespace pyunrealsdk::type_casters
