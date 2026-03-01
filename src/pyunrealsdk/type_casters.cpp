#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/type_casters.h"
#include "pyunrealsdk/exports.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/ffield.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::type_casters {

#ifdef PYUNREALSDK_INTERNAL

namespace {

template <typename SourceType>
const void* downcast_unreal_impl(const SourceType* src, const std::type_info*& type) {
    if (src != nullptr) {
        cast<cast_options<true, true>>(
            src,
            // On successful cast: return the templated type
            [&type]<typename T>(const T* /*obj*/) { type = &typeid(T); },
            // On error: fall back to the source type
            [&type](const SourceType* /*obj*/) { type = &typeid(SourceType); });
    }

    return src;
}

}  // namespace

const void* downcast_unreal(const UObject* src, const std::type_info*& type) {
    return downcast_unreal_impl(src, type);
}
const void* downcast_unreal(const FField* src, const std::type_info*& type) {
    return downcast_unreal_impl(src, type);
}

#endif

PYUNREALSDK_CAPI(PyObject*, cast_from_object, UObject* src);
PYUNREALSDK_CAPI(PyObject*, cast_from_struct, WrappedStruct* src);
PYUNREALSDK_CAPI(PyObject*, cast_from_ffield, FField* src);

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
PYUNREALSDK_CAPI(PyObject*, cast_from_ffield, FField* src) {
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
py::object cast(FField* src) {
    const py::gil_scoped_acquire gil{};
    return py::reinterpret_steal<py::object>(PYUNREALSDK_MANGLE(cast_from_ffield)(src));
}

#endif

PYUNREALSDK_CAPI(UObject*, cast_to_object, PyObject* src);
PYUNREALSDK_CAPI(FField*, cast_to_ffield, PyObject* src);

#ifdef PYUNREALSDK_INTERNAL

// Borrow the reference which was passed into this function, so that we add our own + remove it on
// returning

PYUNREALSDK_CAPI(UObject*, cast_to_object, PyObject* src) {
    const py::gil_scoped_acquire gil{};
    return py::cast<UObject*>(py::reinterpret_borrow<py::object>(src));
}

PYUNREALSDK_CAPI(FField*, cast_to_ffield, PyObject* src) {
    const py::gil_scoped_acquire gil{};
    return py::cast<FField*>(py::reinterpret_borrow<py::object>(src));
}

#else

template <>
UObject* cast<UObject*>(const py::object& src) {
    return PYUNREALSDK_MANGLE(cast_to_object)(src.ptr());
}
template <>
FField* cast<FField*>(const py::object& src) {
    return PYUNREALSDK_MANGLE(cast_to_ffield)(src.ptr());
}

#endif

}  // namespace pyunrealsdk::type_casters
