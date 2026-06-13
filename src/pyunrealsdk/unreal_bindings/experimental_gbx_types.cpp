#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/experimental_gbx_types.h"
#include "pyunrealsdk/base_bindings.h"
#include "pyunrealsdk/stubgen.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "pyunrealsdk/unreal_bindings/wrapped_struct.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/properties/zgbxdefptrproperty.h"
#include "unrealsdk/unreal/properties/zgbxinlinestructproperty.h"
#include "unrealsdk/unreal/structs/fgbxdefptr.h"
#include "unrealsdk/unreal/structs/fgbxinlinestruct.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

namespace {

struct WrappedInlineStruct {
    FGbxInlineStruct inline_struct;
    UScriptStruct* type{};
};

}  // namespace

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void register_experimental_gbx_types(py::module_& mod) {
    PYUNREALSDK_STUBGEN_MODULE_N("unrealsdk.unreal")

    py::classh<FGbxDefPtr>(
        mod, PYUNREALSDK_STUBGEN_CLASS("FGbxDefPtr", ),
        PYUNREALSDK_STUBGEN_DOCSTRING(
            "EXPERIMENTAL TYPE.\n"
            "\n"
            "While the interface is getting more certain, we reserve the right to change it,\n"
            "and break backwards compatibility, at any point.\n"))
        .def_readonly(PYUNREALSDK_STUBGEN_READONLY_PROP("_name", "str"), &FGbxDefPtr::name)
        .def_readonly(PYUNREALSDK_STUBGEN_READONLY_PROP("_type", "UScriptStruct | None"),
                      &FGbxDefPtr::type)
        .def(PYUNREALSDK_STUBGEN_METHOD_N("__init__", "None")
                 py::init([](FName name, const std::variant<UScriptStruct*, std::wstring>& type_arg,
                             const std::optional<bool>& fully_qualified) {
                     UScriptStruct* type_ptr = nullptr;
                     if (!std::holds_alternative<UScriptStruct*>(type_arg)
                         || std::get<UScriptStruct*>(type_arg) != nullptr) {
                         type_ptr = evaluate_scriptstruct_arg(type_arg, fully_qualified);
                     }

                     return FGbxDefPtr(name, type_ptr);
                 }),
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Constructs a new FGbxDefPtr.\n"
                 "\n"
                 "Args:\n"
                 "    name: The name of the entry to resolve.\n"
                 "    type: The type of the entry to resolve, or the type's name.\n"
                 "    fully_qualified: If the type name is fully qualified, or None (the default)\n"
                 "                     to autodetect.\n"
                 "Returns:\n"
                 "    The string representation.\n"),
             PYUNREALSDK_STUBGEN_ARG("name"_a, "str", ),
             PYUNREALSDK_STUBGEN_ARG("type"_a, "UScriptStruct | str | None", "None") = nullptr,
             PYUNREALSDK_STUBGEN_ARG("fully_qualified"_a, "bool | None", "None") = std::nullopt)
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__repr__", "str"),
            [](const FGbxDefPtr& self) {
                auto type_str =
                    self.type == nullptr ? "None" : std::format("'{}'", self.type->Name());
                auto struct_str = (self.type == nullptr || self.instance == nullptr)
                                      ? "None"
                                      : struct_repr(WrappedStruct(self.type, self.instance));

                return std::format("FGbxDefPtr('{}', {}, {})", self.name, type_str, struct_str);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets a string representation of this FGbxDefPtr.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    The string representation.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__dir__", "list[str]"),
            [](const py::object& self) { return py_dir(self, py::cast<FGbxDefPtr*>(self)->type); },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Gets the attributes which exist on the struct behind this pointer.\n"
                "\n"
                "Includes both python attributes and unreal fields. This can be changed to only\n"
                "python attributes by calling dir_includes_unreal.\n"
                "\n"
                "Returns:\n"
                "    A list of attributes which exist on the struct behind this pointer.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__getattr__", "Any"),
            [](const FGbxDefPtr& self, const FName& name) {
                if (self.type == nullptr || self.instance == nullptr) {
                    throw py::attribute_error(
                        std::format("unresolved FGbxDefPtr has no attribute '{}'", name));
                }
                return py_getattr(py_find_field(name, self.type),
                                  reinterpret_cast<uintptr_t>(self.instance), nullptr);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Reads an unreal field off of the struct behind this pointer.\n"
                "\n"
                "Automatically looks up the relevant UField.\n"
                "\n"
                "Args:\n"
                "    name: The name of the field to get.\n"
                "Returns:\n"
                "    The field's value.\n"),
            PYUNREALSDK_STUBGEN_ARG("name"_a, "str", ))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_get_field", "Any"),
            [](const FGbxDefPtr& self, PyFieldVariant::from_py_type field) {
                const PyFieldVariant var{field};
                if (var == nullptr) {
                    throw py::attribute_error("cannot access null attribute");
                }
                if (self.type == nullptr || self.instance == nullptr) {
                    throw py::attribute_error("cannot access field on unresolved FGbxDefPtr");
                }
                return py_getattr(var, reinterpret_cast<uintptr_t>(self.instance), nullptr);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Reads an unreal field off of the struct behind this pointer.\n"
                "\n"
                "In performance critical situations, rather than use __getattr__, you can look up\n"
                "the UField beforehand (via def.type._find()), then pass it directly to this\n"
                "function. This does not get validated, passing a field which doesn't exist on\n"
                "the struct is undefined behaviour.\n"
                "\n"
                "Args:\n"
                "    field: The field to get.\n"
                "Returns:\n"
                "    The field's value.\n"),
#if UNREALSDK_PROPERTIES_ARE_FFIELD
            PYUNREALSDK_STUBGEN_ARG("field"_a, "UField | ZProperty", )
#else
            PYUNREALSDK_STUBGEN_ARG("field"_a, "UField", )
#endif
                )
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__setattr__", "None"),
            [](FGbxDefPtr& self, const py::str& name, const py::object& value) {
                // See if the standard setattr would work first, in case we're being called on an
                // existing field. Getattr is only called on failure, but setattr is always called.
                PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> storage;
                auto& setattr = storage
                                    .call_once_and_store_result([]() {
                                        return py::module::import("builtins")
                                            .attr("object")
                                            .attr("__setattr__");
                                    })
                                    .get_stored();

                try {
                    setattr(self, name, value);
                    return;
                } catch (py::error_already_set& e) {
                    if (!e.matches(PyExc_AttributeError)) {
                        throw;
                    }
                }

                auto fname = py::cast<FName>(name);
                if (self.type == nullptr || self.instance == nullptr) {
                    throw py::attribute_error(
                        std::format("unresolved FGbxDefPtr has no attribute '{}'", fname));
                }
                py_setattr_direct(py_find_field(fname, self.type),
                                  reinterpret_cast<uintptr_t>(self.instance), value);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Writes a value to an unreal field on the struct behind this pointer.\n"
                "\n"
                "Automatically looks up the relevant UField.\n"
                "\n"
                "Args:\n"
                "    name: The name of the field to set.\n"
                "    value: The value to write.\n"),
            PYUNREALSDK_STUBGEN_ARG("name"_a, "str", ), PYUNREALSDK_STUBGEN_ARG("value"_a, "Any", ))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_set_field", "None"),
            [](FGbxDefPtr& self, PyFieldVariant::from_py_type field, const py::object& value) {
                const PyFieldVariant var{field};
                if (var == nullptr) {
                    throw py::attribute_error("cannot access null attribute");
                }
                if (self.type == nullptr || self.instance == nullptr) {
                    throw py::attribute_error("cannot access field on unresolved FGbxDefPtr");
                }
                py_setattr_direct(var, reinterpret_cast<uintptr_t>(self.instance), value);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Writes a value to an unreal field on the struct behind this pointer.\n"
                "\n"
                "In performance critical situations, rather than use __setattr__, you can look up\n"
                "the UField beforehand (via def.type._find()), then pass it directly to this\n"
                "function. This does not get validated, passing a field which doesn't exist on\n"
                "the struct is undefined behaviour.\n"
                "\n"
                "Args:\n"
                "    field: The field to set.\n"
                "    value: The value to write.\n"),
#if UNREALSDK_PROPERTIES_ARE_FFIELD
            PYUNREALSDK_STUBGEN_ARG("field"_a, "UField | ZProperty", ),
#else
            PYUNREALSDK_STUBGEN_ARG("field"_a, "UField", ),
#endif
            PYUNREALSDK_STUBGEN_ARG("value"_a, "Any", ))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_get_address", "int"),
            [](const FGbxDefPtr& self) { return reinterpret_cast<uintptr_t>(self.instance); },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Gets the address of this pointer's resolved instance, for debugging.\n"
                "\n"
                "Returns:\n"
                "    This instance's address.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_experimental_alloc_and_mem_leak", "None"),
            [](FGbxDefPtr& self) {
                if (self.instance != nullptr) {
                    throw std::invalid_argument(
                        "Cannot allocate a FGbxDefPtr which has already been resolved!");
                }
                if (self.type == nullptr) {
                    throw std::invalid_argument("Cannot allocate a FGbxDefPtr without a type!");
                }
                self.instance = unrealsdk::u_malloc(self.type->get_struct_size());
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "If this pointer is not already resolved, try allocate memory for it.\n"
                "\n"
                "Untested, expected to cause a memory leak.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_experimental_free_and_use_after_free", "None"),
            [](FGbxDefPtr& self) {
                if (self.instance == nullptr) {
                    throw std::invalid_argument(
                        "Cannot free a FGbxDefPtr which has not been resolved!");
                }
                void* ptr = nullptr;
                std::swap(self.instance, ptr);
                unrealsdk::u_free(ptr);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "If this pointer is resolved, try free its memory.\n"
                "\n"
                "Untested, expected to cause a use-after free and/or a double-free.\n"));

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
