#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "pyunrealsdk/static_py_object.h"
#include "pyunrealsdk/unreal_bindings/uenum.h"
#include "pyunrealsdk/unreal_bindings/wrapped_array.h"
#include "pyunrealsdk/unreal_bindings/wrapped_struct.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"
#include "unrealsdk/unreal/classes/uconst.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/classes/ustruct_funcs.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

namespace {

bool dir_includes_unreal = true;

}  // namespace

UField* py_find_field(const FName& name, const UStruct* type) {
    try {
        return type->find(name);
    } catch (const std::invalid_argument&) {
        throw py::attribute_error(
            unrealsdk::fmt::format("'{}' object has no attribute '{}'", type->Name, name));
    }
}

void register_property_helpers(py::module_& mod) {
    mod.def(
        "dir_includes_unreal", [](bool should_include) { dir_includes_unreal = should_include; },
        "Sets if `__dir__` should include dynamic unreal properties, specific to the\n"
        "object. Defaults to true.\n"
        "\n"
        "Args:\n"
        "    should_include: True if to include dynamic properties, false to not.",
        "should_include"_a);
}

std::vector<std::string> py_dir(const py::object& self, const UStruct* type) {
    // Start by calling the base dir function
    PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> storage;
    auto& dir = storage
                    .call_once_and_store_result([]() {
                        return py::module_::import("builtins").attr("object").attr("__dir__");
                    })
                    .get_stored();

    auto names = py::cast<std::vector<std::string>>(dir(self));

    if (dir_includes_unreal) {
        // Append our fields
        auto fields = type->fields();
        std::ranges::transform(fields, std::back_inserter(names),
                               [](auto obj) { return obj->Name; });
    }

    return names;
}

py::object py_getattr(UField* field,
                      uintptr_t base_addr,
                      const unrealsdk::unreal::UnrealPointer<void>& parent,
                      UObject* func_obj) {
    if (field->is_instance(find_class<UProperty>())) {
        auto prop = reinterpret_cast<UProperty*>(field);
        if (prop->ArrayDim < 1) {
            throw py::attribute_error(unrealsdk::fmt::format("attribute '{}' has size of {}",
                                                             prop->Name, prop->ArrayDim));
        }

        // If we have a static array, return it as a tuple.
        // Store in a list for now so we can still append.
        py::list ret{prop->ArrayDim};

        cast(prop, [base_addr, &ret, &parent]<typename T>(const T* prop) {
            for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
                auto val = get_property<T>(prop, i, base_addr, parent);

                // Multiple property types expose a get enum method
                constexpr bool is_enum = requires(const T* type) {
                    { type->get_enum() } -> std::same_as<UEnum*>;
                };

                // If the value we're reading is an enum, convert it to a python enum
                if constexpr (is_enum) {
                    auto ue_enum = prop->get_enum();
                    if (ue_enum != nullptr) {
                        ret[i] = enum_as_py_enum(ue_enum)(val);
                        continue;
                    }
                }
                // Otherwise store as is

                ret[i] = std::move(val);
            }
        });
        if (prop->ArrayDim == 1) {
            return ret[0];
        }
        return py::tuple(ret);
    }
    if (field->is_instance(find_class<UFunction>())) {
        if (func_obj == nullptr) {
            throw py::attribute_error(
                unrealsdk::fmt::format("cannot bind function '{}' with null object", field->Name));
        }

        return py::cast(
            BoundFunction{.func = reinterpret_cast<UFunction*>(field), .object = func_obj});
    }

    if (field->is_instance(find_class<UScriptStruct>())) {
        return py::cast(field);
    }

    if (field->is_instance(find_class<UConst>())) {
        return py::cast((std::string) reinterpret_cast<UConst*>(field)->Value);
    }

    if (field->is_instance(find_class<UEnum>())) {
        return enum_as_py_enum(reinterpret_cast<UEnum*>(field));
    }

    throw py::attribute_error(unrealsdk::fmt::format("attribute '{}' has unknown type '{}'",
                                                     field->Name, field->Class->Name));
}

// The templated lambda and all the if constexprs make everything have a really high penalty
// Yes it's probably a bit complex, but it's also a bit awkward trying to split it up
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void py_setattr_direct(UField* field, uintptr_t base_addr, const py::object& value) {
    if (!field->is_instance(find_class<UProperty>())) {
        throw py::attribute_error(unrealsdk::fmt::format(
            "attribute '{}' is not a property, and thus cannot be set", field->Name));
    }

    auto prop = reinterpret_cast<UProperty*>(field);

    py::sequence value_seq;
    if (prop->ArrayDim > 1) {
        if (!py::isinstance<py::sequence>(value)) {
            std::string value_type_name = py::str(py::type::of(value).attr("__name__"));
            throw py::type_error(unrealsdk::fmt::format(
                "attribute value has unexpected type '{}', expected a sequence", value_type_name));
        }
        value_seq = value;

        if (value_seq.size() > static_cast<size_t>(prop->ArrayDim)) {
            throw py::type_error(unrealsdk::fmt::format(
                "attribute value is too long, {} supports a maximum of {} values", prop->Name,
                prop->ArrayDim));
        }
    } else {
        value_seq = py::make_tuple(value);
    }

    cast(prop, [base_addr, &value_seq]<typename T>(const T* prop) {
        using value_type = typename PropTraits<T>::Value;

        const size_t seq_size = value_seq.size();
        const size_t prop_size = prop->ArrayDim;

        // As a special case, if we have an array property, allow assigning non-wrapped array
        // sequences
        if constexpr (std::is_same_v<T, UArrayProperty>) {
            // Also make sure it's not somehow a fixed array, since the sdk can't handle that, let
            // it fall through to the standard error handler
            if (prop_size == 1 && seq_size == 1 && !py::isinstance<WrappedArray>(value_seq[0])
                && py::isinstance<py::sequence>(value_seq[0])) {
                // Implement using slice assignment
                auto arr = get_property<UArrayProperty>(prop, 0, base_addr);
                impl::array_py_setitem_slice(
                    arr, py::slice(std::nullopt, std::nullopt, std::nullopt), value_seq[0]);
                return;
            }
        }

        // If we're default constructable, set all the missing fields to the default value
        // If we're not, require specifying all values
        // Do this before writing the given values so that we can error out without making changes
        if constexpr (std::is_default_constructible_v<value_type>) {
            for (size_t i = seq_size; i < prop_size; i++) {
                set_property<T>(prop, i, base_addr, {});
            }
        } else {
            if (seq_size != prop_size) {
                throw py::type_error(unrealsdk::fmt::format(
                    "attribute value is too short, {} must be given as exactly {} values (no known "
                    "default to use when less are given)",
                    prop->Name, prop->ArrayDim));
            }
        }

        for (size_t i = 0; i < seq_size; i++) {
            // If we're setting a struct property, we might be being told to ignore it
            if constexpr (std::is_base_of_v<UStructProperty, T>) {
                if (is_ignore_struct_sentinel(value_seq[i])) {
                    continue;
                }
            }

            set_property<T>(prop, i, base_addr, py::cast<value_type>(value_seq[i]));
        }
    });
}

}  // namespace pyunrealsdk::unreal

#endif
