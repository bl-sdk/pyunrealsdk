#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/unreal/cast_prop.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/classes/ustruct_funcs.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

namespace {

bool dir_includes_unreal = true;

/**
 * @brief Gets a field off of an object based on the key given to getattr/setattr.
 * @note Allows both strings and direct field references.
 *
 * @param key The python key.
 * @param type The type of the unreal object this access is reading off of.
 * @return The field. Invalid keys throw, so will never be null.
 */
UField* get_field_from_py_key(const py::object& key, const UStruct* type) {
    if (py::isinstance<py::str>(key)) {
        std::string key_str = py::str(key);
        try {
            return type->find(key_str);
        } catch (const std::invalid_argument&) {
            throw py::attribute_error(
                unrealsdk::fmt::format("'{}' object has no attribute '{}'", type->Name, key_str));
        }
    }

    if (py::isinstance<UField>(key)) {
        auto field = py::cast<UField*>(key);
        if (field == nullptr) {
            throw py::attribute_error("cannot access null attribute");
        }
        return field;
    }

    std::string key_type_name = py::str(py::type::of(key).attr("__name__"));
    throw py::attribute_error(
        unrealsdk::fmt::format("attribute key has unknown type '{}'", key_type_name));
}

}  // namespace

void register_property_helpers(py::module_& mod) {
    mod.def(
        "dir_includes_unreal", [](bool should_include) { dir_includes_unreal = should_include; },
        "Sets if `__dir__` should include dynamic unreal properties, specific to the"
        "object. Defaults to true.\n"
        "\n",
        "Args:\n"
        "    should_include: True if to include dynamic properties, false to not.\n",
        "should_include"_a);
}

std::vector<std::string> py_dir(const py::object& self, const UStruct* type) {
    // Start by calling the base dir function
    auto names = py::cast<std::vector<std::string>>(
        py::module_::import("builtins").attr("object").attr("__dir__")(self));

    if (dir_includes_unreal) {
        // Append our fields
        auto fields = type->fields();
        std::transform(fields.begin(), fields.end(), std::back_inserter(names),
                       [](auto obj) { return obj->Name; });
    }

    return names;
}

py::object py_getattr(uintptr_t base_addr,
                      const UStruct* type,
                      const py::object& key,
                      UObject* func_obj) {
    // We can't push these at a higher scope because we need them to only run after the
    // sdk's been initialized
    static const UClass* uproperty_class = find_class(L"Property"_fn);
    static const UClass* ufunction_class = find_class(L"Function"_fn);

    auto field = get_field_from_py_key(key, type);

    if (field->is_instance(uproperty_class)) {
        auto prop = reinterpret_cast<UProperty*>(field);
        if (prop->ArrayDim < 1) {
            throw py::attribute_error(unrealsdk::fmt::format("attribute '{}' has size of {}",
                                                             prop->Name, prop->ArrayDim));
        }

        // If we have a static array, return it as a tuple.
        // Store in a list for now so we can still append.
        py::list ret{prop->ArrayDim};

        cast_prop(prop, [base_addr, &ret]<typename T>(const T* prop) {
            for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
                ret[i] = get_property<T>(prop, i, base_addr);
            }
        });
        if (prop->ArrayDim == 1) {
            return ret[0];
        }
        return py::tuple(ret);
    }
    if (field->is_instance(ufunction_class)) {
        if (func_obj == nullptr) {
            throw py::attribute_error(
                unrealsdk::fmt::format("cannot bind function '{}' with null object", field->Name));
        }

        return py::cast(BoundFunction{reinterpret_cast<UFunction*>(field), func_obj});
    }

    throw py::attribute_error(unrealsdk::fmt::format("attribute '{}' has unknown type '{}'",
                                                     field->Name, field->Class->Name));
}

void py_setattr(uintptr_t base_addr,
                const UStruct* type,
                const py::object& key,
                const py::object& value) {
    // We can't put this at a higher scope because we need it to only run after the
    // sdk's been initialized
    static const UClass* uproperty_class = find_class(L"Property"_fn);

    UField* field = get_field_from_py_key(key, type);
    if (field->is_instance(uproperty_class)) {
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

    cast_prop(prop, [base_addr, &value_seq]<typename T>(const T* prop) {
        using value_type = typename PropTraits<T>::Value;

        size_t seq_size = value_seq.size();
        size_t prop_size = prop->ArrayDim;

        // If we're default constructable, set all the missing fields to the default value
        // If we're not, require specifying all values
        // Do this before writing the given values so that we can error out without making changes
        if constexpr (std::is_default_constructible_v<value_type>) {
            for (size_t i = seq_size; i < prop_size; i++) {
                set_property(prop, i, base_addr, {});
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
            set_property(prop, i, base_addr, py::cast<value_type>(value_seq[i]));
        }
    });
}

}  // namespace pyunrealsdk::unreal
