#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/weak_pointer.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/wrappers/weak_pointer.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_weak_pointer(py::module_& mod) {
    auto cls = py::class_<WeakPointer>(mod, "WeakPointer");
    cls.def(py::init<const UObject*>(),
            "Creates a weak reference to an unreal object.\n"
            "\n"
            "Under Unreal 3 this is emulated, as there's no built in support for weak\n"
            "references. This means there's a very rare chance that this returns a different\n"
            "object than what was it was set to, however it will be near identical, and it\n"
            "will always be a valid object.\n"
            "\n"
            "Args:\n"
            "    obj: The object to create a weak reference to.",
            "obj"_a = nullptr)
        .def(
            "__call__",
            // Take a mutable reference, since the mutable dereference can do some optimizations
            [](WeakPointer& self) { return *self; },
            "Gets the object this is pointing at.\n"
            "\n"
            "Note that there's no way to get a strong reference to an unreal object. This\n"
            "means if you're using this on a thread, it's always possible for the engine to\n"
            "pull the object out from under you after you retrieve it. However, it *should*\n"
            "be safe under a hook, since the GC shouldn't be running.\n"
            "\n"
            "Returns:\n"
            "    The object this is pointing at, or None if it's become invalid.");

    // Create as a class method, see pybind11#1693
    cls.attr("__class_getitem__") = py::reinterpret_borrow<py::object>(PyClassMethod_New(
        py::cpp_function([](const py::type& cls, const py::args& /*args*/,
                            const py::kwargs& /*kwargs*/) { return cls; },
                         "No-op, implemented to allow type stubs to treat this as a generic type.\n"
                         "\n"
                         "Args:\n"
                         "    *args: Ignored.\n"
                         "    **kwargs: Ignored.\n"
                         "Returns:\n"
                         "    The WeakPointer class.",
                         "cls"_a)
            .ptr()));
}

}  // namespace pyunrealsdk::unreal
#endif
