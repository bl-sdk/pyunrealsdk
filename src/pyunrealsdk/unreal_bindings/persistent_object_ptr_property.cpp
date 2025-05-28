#include "pyunrealsdk/pch.h"
#include "unrealsdk/unreal/classes/properties/persistent_object_ptr_property.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/persistent_object_ptr_property.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/structs/tpersistentobjectptr.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

namespace {

/**
 * @brief Call the relevant `get_from` overload given the args passed to the python function.
 * @note Also ensures no null pointers.
 *
 * @tparam PathType The type of the path to get.
 * @tparam PropertyType The type of the property associated with this path.
 * @param source The source location.
 * @param prop The property to get.
 * @param idx The fixed array index.
 * @return A pointer to the property's identifier.
 */
template <typename PathType, typename PropertyType>
const PathType* resolve_get_from_overload(
    const std::variant<const UObject*, const WrappedStruct*>& source,
    const std::variant<FName, const PropertyType*>& prop,
    size_t idx) {
    if (std::holds_alternative<const UObject*>(source)) {
        auto source_ptr = std::get<const UObject*>(source);
        if (source_ptr == nullptr) {
            throw std::invalid_argument("Cannot get identifier from a null source!");
        }

        if (std::holds_alternative<FName>(prop)) {
            return PathType::get_from(source_ptr, std::get<FName>(prop), idx);
        }

        auto prop_ptr = std::get<const PropertyType*>(prop);
        if (prop_ptr == nullptr) {
            throw std::invalid_argument("Cannot get identifier using a null property!");
        }
        return PathType::get_from(source_ptr, prop_ptr, idx);
    }

    auto source_ptr = std::get<const WrappedStruct*>(source);
    if (source_ptr == nullptr) {
        throw std::invalid_argument("Cannot get identifier from a null source!");
    }

    if (std::holds_alternative<FName>(prop)) {
        return PathType::get_from(*source_ptr, std::get<FName>(prop), idx);
    }

    auto prop_ptr = std::get<const PropertyType*>(prop);
    if (prop_ptr == nullptr) {
        throw std::invalid_argument("Cannot get identifier using a null property!");
    }
    return PathType::get_from(*source_ptr, prop_ptr, idx);
}

/**
 * @brief Converts a lazy object path to it's python equivalent.
 *
 * @param path The path to convert.
 * @return The python representation of the lazy object path.
 */
py::bytes lazy_obj_path_to_py(const FLazyObjectPath* path) {
    // Just return the raw guid bytes for now, since it's probably the most neutral way
    // of doing it, and this isn't likely to be used much anyway

    // Can't easily return a `Guid` struct since there are plenty of structs using that
    // name, and the package of the core engine one we want changes between versions, so
    // we can't really easily fully qualify it either

    return {reinterpret_cast<const char*>(path), sizeof(*path)};
}

/**
 * @brief Converts a soft object path to it's python equivalent.
 *
 * @param path The path to convert.
 * @return The python representation of the soft object path.
 */
std::wstring soft_obj_path_to_py(const FSoftObjectPath* path) {
    std::wstring name{path->asset_path_name};
    if (path->subpath.size() > 0) {
        name.reserve(name.size() + path->subpath.size() + 1);
        name += L':';
        name += (std::wstring_view)path->subpath;
    }
    return name;
}

}  // namespace

void register_persistent_object_properties(py::module_& mod) {
    PyUEClass<ULazyObjectProperty, UObjectProperty>(mod, "ULazyObjectProperty")
        .def_static(
            "_get_identifier_from",
            [](const std::variant<const UObject*, const WrappedStruct*>& source,
               const std::variant<FName, const ULazyObjectProperty*>& prop, size_t idx) {
                auto path = resolve_get_from_overload<FLazyObjectPath, ULazyObjectProperty>(
                    source, prop, idx);
                return lazy_obj_path_to_py(path);
            },
            "Gets the Guid identifier associated with a given lazy object property.\n"
            "\n"
            "When using standard attribute access, lazy object properties resolve directly to\n"
            "their contained object. This function can be used to get the identifier instead.\n"
            "\n"
            "Args:\n"
            "    source: The object or struct holding the property to get.\n"
            "    prop: The lazy object property, or name thereof, to get.\n"
            "    idx: If this property is a fixed sized array, which index to get.\n"
            "Returns:\n"
            "    The raw 16 bytes composing the property's Guid.",
            "source"_a, "prop"_a, "idx"_a = 0)
        .def_static(
            "_get_identifier_from_array",
            [](const WrappedArray& source, size_t idx) {
                auto path = FLazyObjectPath::get_from_array(source, idx);
                return lazy_obj_path_to_py(path);
            },
            "Gets the Guid identifier associated with a given lazy object property.\n"
            "\n"
            "When using standard attribute access, lazy object properties resolve directly to\n"
            "their contained object. This function can be used to get the identifier instead.\n"
            "\n"
            "Args:\n"
            "    source: The array holding the property to get.\n"
            "    idx: The index into the array to get from.\n"
            "Returns:\n"
            "    The raw 16 bytes composing the property's Guid.",
            "source"_a, "idx"_a);

    PyUEClass<USoftObjectProperty, UObjectProperty>(mod, "USoftObjectProperty")
        .def_static(
            "_get_identifier_from",
            [](const std::variant<const UObject*, const WrappedStruct*>& source,
               const std::variant<FName, const USoftObjectProperty*>& prop, size_t idx) {
                auto path = resolve_get_from_overload<FSoftObjectPath, USoftObjectProperty>(
                    source, prop, idx);
                return soft_obj_path_to_py(path);
            },
            "Gets the path name identifier associated with a given soft object property.\n"
            "\n"
            "When using standard attribute access, soft object properties resolve directly to\n"
            "their contained object. This function can be used to get the identifier instead.\n"
            "\n"
            "Args:\n"
            "    source: The object or struct holding the property to get.\n"
            "    prop: The soft object property, or name thereof, to get.\n"
            "    idx: If this property is a fixed sized array, which index to get.\n"
            "Returns:\n"
            "    The path name of the object the given property is looking for.",
            "source"_a, "prop"_a, "idx"_a = 0)
        .def_static(
            "_get_identifier_from_array",
            [](const WrappedArray& source, size_t idx) {
                auto path = FSoftObjectPath::get_from_array(source, idx);
                return soft_obj_path_to_py(path);
            },
            "Gets the path name identifier associated with a given soft object property.\n"
            "\n"
            "When using standard attribute access, soft object properties resolve directly to\n"
            "their contained object. This function can be used to get the identifier instead.\n"
            "\n"
            "Args:\n"
            "    source: The array holding the property to get.\n"
            "    idx: The index into the array to get from.\n"
            "Returns:\n"
            "    The path name of the object the given property is looking for.",
            "source"_a, "idx"_a);

    PyUEClass<USoftClassProperty, USoftObjectProperty>(mod, "USoftClassProperty")
        .def_member_prop("MetaClass", &USoftClassProperty::MetaClass);
}

}  // namespace pyunrealsdk::unreal
#endif
