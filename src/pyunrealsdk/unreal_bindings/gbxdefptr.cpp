#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/gbxdefptr.h"
#include "pyunrealsdk/stubgen.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/properties/zgbxdefptrproperty.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_gbxdefptr(py::module_& mod) {
    PYUNREALSDK_STUBGEN_MODULE_N("unrealsdk.unreal")

    py::classh<FGbxDefPtr>(
        mod, PYUNREALSDK_STUBGEN_CLASS("GbxDefPtr", ),
        PYUNREALSDK_STUBGEN_DOCSTRING(
            "EXPERIMENTAL TYPE.\n"
            "\n"
            "It may currently leak memory and/or cause crashes, and it's semantics are all\n"
            "liable to change, it might even get removed.\n"))
        .def(py::init<>() PYUNREALSDK_STUBGEN_METHOD_N("__init__", "None"),
             PYUNREALSDK_STUBGEN_DOCSTRING("Creates a default (empty) GbxDefPtr."))
        .def_readwrite(PYUNREALSDK_STUBGEN_ATTR("_experimental_name", "str"), &FGbxDefPtr::name)
        .def_readwrite(PYUNREALSDK_STUBGEN_ATTR("_experimental_ref", "UScriptStruct"),
                       &FGbxDefPtr::ref)
        .def_property_readonly(
            PYUNREALSDK_STUBGEN_READONLY_PROP("_experimental_instance", "WrappedStruct | None"),
            [](const FGbxDefPtr& self) -> std::optional<WrappedStruct> {
                if (self.ref == nullptr || self.instance == nullptr) {
                    return std::nullopt;
                }
                return WrappedStruct{self.ref, self.instance};
            })
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__repr__", "str"),
            [](const FGbxDefPtr& self) {
                return std::format(
                    "GbxDefPtr('{}', {})", self.name,
                    (self.ref == nullptr || self.instance == nullptr) ? "None" : "{...}");
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets a string representation of this GbxDefPtr.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    The string representation.\n"));
}

}  // namespace pyunrealsdk::unreal

#endif
