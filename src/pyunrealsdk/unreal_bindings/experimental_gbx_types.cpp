#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/experimental_gbx_types.h"
#include "pyunrealsdk/stubgen.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/properties/zgbxdefptrproperty.h"
#include "unrealsdk/unreal/properties/zgbxinlinestructproperty.h"
#include "unrealsdk/unreal/structs/fgbxdefptr.h"
#include "unrealsdk/unreal/structs/fgbxinlinestruct.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

namespace {

struct WrappedInlineStruct {
    FGbxInlineStruct inline_struct;
    UScriptStruct* type{};
};

}  // namespace

void register_experimental_gbx_types(py::module_& mod) {
    PYUNREALSDK_STUBGEN_MODULE_N("unrealsdk.unreal")

    py::classh<FGbxDefPtr>(
        mod, PYUNREALSDK_STUBGEN_CLASS("FGbxDefPtr", ),
        PYUNREALSDK_STUBGEN_DOCSTRING(
            "EXPERIMENTAL TYPE.\n"
            "\n"
            "It may currently leak memory and/or cause crashes, and it's semantics are all\n"
            "liable to change, it might even get removed.\n"))
        .def(py::init<>() PYUNREALSDK_STUBGEN_METHOD_N("__init__", "None"),
             PYUNREALSDK_STUBGEN_DOCSTRING("Creates a default (empty) FGbxDefPtr."))
        .def_readwrite(PYUNREALSDK_STUBGEN_ATTR("_experimental_name", "str"), &FGbxDefPtr::name)
        .def_readwrite(PYUNREALSDK_STUBGEN_ATTR("_experimental_ref", "UScriptStruct | None"),
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
                    "FGbxDefPtr('{}', {})", self.name,
                    (self.ref == nullptr || self.instance == nullptr) ? "None" : "{...}");
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets a string representation of this FGbxDefPtr.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    The string representation.\n"));

    py::classh<WrappedInlineStruct>(
        mod, PYUNREALSDK_STUBGEN_CLASS("WrappedInlineStruct", ),
        PYUNREALSDK_STUBGEN_DOCSTRING(
            "EXPERIMENTAL TYPE.\n"
            "\n"
            "It may currently leak memory and/or cause crashes, and it's semantics are all\n"
            "liable to change, it might even get removed.\n"))
        .def_readwrite(PYUNREALSDK_STUBGEN_ATTR("_experimental_type", "UScriptStruct"),
                       &WrappedInlineStruct::type)
        .def_property(
            PYUNREALSDK_STUBGEN_ATTR("_experimental_flags", "str"),
            [](WrappedInlineStruct& self) { return self.inline_struct.flags; },
            [](WrappedInlineStruct& self, decltype(FGbxInlineStruct::flags) flags) {
                self.inline_struct.flags = flags;
            })
        .def_property_readonly(
            PYUNREALSDK_STUBGEN_READONLY_PROP("_experimental_instance", "WrappedStruct | None"),
            [](const WrappedInlineStruct& self) -> std::optional<WrappedStruct> {
                if (self.type == nullptr || self.inline_struct.instance == nullptr) {
                    return std::nullopt;
                }
                return WrappedStruct{self.type, self.inline_struct.instance};
            });
}

py::object convert_gbx_inline_struct_prop(const ZGbxInlineStructProperty* prop,
                                          FGbxInlineStruct* inline_struct) {
    return py::cast(
        WrappedInlineStruct{.inline_struct = *inline_struct, .type = prop->MetaStruct()});
}

}  // namespace pyunrealsdk::unreal

#endif
