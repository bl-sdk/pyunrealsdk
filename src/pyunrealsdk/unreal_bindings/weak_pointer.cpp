#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/weak_pointer.h"
#include "pyunrealsdk/stubgen.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/wrappers/weak_pointer.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_weak_pointer(py::module_& mod) {
    PYUNREALSDK_STUBGEN_MODULE_N("unrealsdk.unreal")

#if UNREALSDK_HAS_NATIVE_WEAK_POINTERS
    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EMULATED_WEAK_POINTER_NOTICE
#else
    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EMULATED_WEAK_POINTER_NOTICE                                                     \
    "This is emulated, as there's no built in support for weak references. This means\n" \
    "there's a very rare chance that this returns a different object than what it was\n" \
    "set to. Even if this happens, it will always return a valid object, and it will\n"  \
    "be a near-identical one.\n"                                                         \
    "\n"
#endif

    auto cls =
        py::classh<WeakPointer>(mod, PYUNREALSDK_STUBGEN_CLASS("WeakPointer", )
                                         PYUNREALSDK_STUBGEN_GENERIC_N("[T: UObject = UObject]"));
    cls.def(py::init<const UObject*>() PYUNREALSDK_STUBGEN_METHOD_N("__init__", "None"),
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Creates a weak reference to an unreal object.\n" /* alignment */
                "\n"                                              /* alignment */
                EMULATED_WEAK_POINTER_NOTICE                      /* alignment */
                "Args:\n"
                "    obj: The object to create a weak reference to.\n"),
            PYUNREALSDK_STUBGEN_ARG("obj"_a, "UObject | None", "None") = nullptr)
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__call__", "T | None"),
            // Take a mutable reference, since the mutable dereference can do some optimizations
            [](WeakPointer& self) { return *self; },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Gets the object this is pointing at.\n"
                "\n"
                "Note that there's no way to get a strong reference to an unreal object. This\n"
                "means if you're using this on a thread, it's always possible for the engine to\n"
                "pull the object out from under you after you retrieve it. However, it *should*\n"
                "be safe under a hook, since the GC shouldn't be running.\n"
                "\n"
                "Returns:\n"
                "    The object this is pointing at, or None if it's become invalid.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("replace", "None"),
            [](WeakPointer& self, const UObject* obj) { self = WeakPointer{obj}; },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Replaces the reference in this pointer in-place.\n"
                "\n"
                "This is equivalent to assigning the same variable to a new pointer, but may be\n"
                "more convenient when modifying a parent scope.\n"
                "\n"
                "Args:\n"
                "    obj: The new object to hold a weak reference to.\n"),
            PYUNREALSDK_STUBGEN_ARG("obj"_a, "T | None", ));

    // Create as a class method, see pybind11#1693
    cls.attr(PYUNREALSDK_STUBGEN_CLASSMETHOD("__class_getitem__", "GenericAlias")) =
        py::reinterpret_borrow<py::object>(PyClassMethod_New(
            py::cpp_function(
                [](const py::type& cls, const py::args& /*args*/, const py::kwargs& /*kwargs*/) {
                    return cls;
                },
                PYUNREALSDK_STUBGEN_DOCSTRING(
                    "No-op, implemented to allow type stubs to treat this as a generic type.\n"
                    "\n"
                    "Args:\n"
                    "    *args: Ignored.\n"
                    "    **kwargs: Ignored.\n"
                    "Returns:\n"
                    "    The WeakPointer class.\n"),
                "cls"_a)
                .ptr()));
    PYUNREALSDK_STUBGEN_ARG_N("*args"_a, "Any", )
    PYUNREALSDK_STUBGEN_ARG_N("**kwargs"_a, "Any", )
}

}  // namespace pyunrealsdk::unreal
#endif
