#include "pyunrealsdk/pch.h"
#include "unrealsdk/unreal/properties/persistent_object_ptr_property.h"
#include "pyunrealsdk/stubgen.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/persistent_object_ptr_property.h"
#include "unrealsdk/unreal/properties/zobjectproperty.h"
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
    PYUNREALSDK_STUBGEN_MODULE_N("unrealsdk.unreal")

    PyUEClass<ZLazyObjectProperty, ZObjectProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZLazyObjectProperty", "ZObjectProperty"))
        .def_static(
            PYUNREALSDK_STUBGEN_STATICMETHOD("_get_identifier_from", "bytes"),
            [](const std::variant<const UObject*, const WrappedStruct*>& source,
               const std::variant<FName, const ZLazyObjectProperty*>& prop, size_t idx) {
                auto path = resolve_get_from_overload<FLazyObjectPath, ZLazyObjectProperty>(
                    source, prop, idx);
                return lazy_obj_path_to_py(path);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
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
                "    The raw 16 bytes composing the property's Guid.\n"),
            PYUNREALSDK_STUBGEN_ARG("source"_a, "UObject | WrappedStruct", ),
            PYUNREALSDK_STUBGEN_ARG("prop"_a, "ZLazyObjectProperty | str", ),
            PYUNREALSDK_STUBGEN_ARG("idx"_a, "int", "0") = 0)
        .def_static(
            PYUNREALSDK_STUBGEN_STATICMETHOD("_get_identifier_from_array", "bytes"),
            [](const WrappedArray& source, size_t idx) {
                auto path = FLazyObjectPath::get_from_array(source, idx);
                return lazy_obj_path_to_py(path);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Gets the Guid identifier associated with a given lazy object property.\n"
                "\n"
                "When using standard attribute access, lazy object properties resolve directly to\n"
                "their contained object. This function can be used to get the identifier instead.\n"
                "\n"
                "Args:\n"
                "    source: The array holding the property to get.\n"
                "    idx: The index into the array to get from.\n"
                "Returns:\n"
                "    The raw 16 bytes composing the property's Guid.\n"),
            PYUNREALSDK_STUBGEN_ARG("source"_a, "WrappedArray", ),
            PYUNREALSDK_STUBGEN_ARG("idx"_a, "int", ));

    PyUEClass<ZSoftObjectProperty, ZObjectProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZSoftObjectProperty", "ZObjectProperty"))
        .def_static(
            PYUNREALSDK_STUBGEN_STATICMETHOD("_get_identifier_from", "str"),
            [](const std::variant<const UObject*, const WrappedStruct*>& source,
               const std::variant<FName, const ZSoftObjectProperty*>& prop, size_t idx) {
                auto path = resolve_get_from_overload<FSoftObjectPath, ZSoftObjectProperty>(
                    source, prop, idx);
                return soft_obj_path_to_py(path);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
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
                "    The path name of the object the given property is looking for.\n"),
            PYUNREALSDK_STUBGEN_ARG("source"_a, "UObject | WrappedStruct", ),
            PYUNREALSDK_STUBGEN_ARG("prop"_a, "ZSoftObjectProperty | str", ),
            PYUNREALSDK_STUBGEN_ARG("idx"_a, "int", "0") = 0)
        .def_static(
            PYUNREALSDK_STUBGEN_STATICMETHOD("_get_identifier_from_array", "str"),
            [](const WrappedArray& source, size_t idx) {
                auto path = FSoftObjectPath::get_from_array(source, idx);
                return soft_obj_path_to_py(path);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Gets the path name identifier associated with a given soft object property.\n"
                "\n"
                "When using standard attribute access, soft object properties resolve directly to\n"
                "their contained object. This function can be used to get the identifier instead.\n"
                "\n"
                "Args:\n"
                "    source: The array holding the property to get.\n"
                "    idx: The index into the array to get from.\n"
                "Returns:\n"
                "    The path name of the object the given property is looking for.\n"),
            PYUNREALSDK_STUBGEN_ARG("source"_a, "WrappedArray", ),
            PYUNREALSDK_STUBGEN_ARG("idx"_a, "int", ));

    PyUEClass<ZSoftClassProperty, ZSoftObjectProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZSoftClassProperty", "ZSoftObjectProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("MetaClass", "UClass"),
                         &ZSoftClassProperty::MetaClass);

    // Deprecated UProperty Aliases - Same as in uobject_children.cpp

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_DEPRECATED_PROPERTY_ALIAS(name_without_prefix, parent_class)      \
    PYUNREALSDK_STUBGEN_CLASS_N("U" name_without_prefix,                          \
                                "Z" name_without_prefix ", " parent_class)        \
    PYUNREALSDK_STUBGEN_DEPRECATED_N("U" name_without_prefix                      \
                                     " has been renamed to Z" name_without_prefix \
                                     ", this is a deprecated alias")              \
    mod.attr("U" name_without_prefix) = mod.attr("Z" name_without_prefix);

    DECLARE_DEPRECATED_PROPERTY_ALIAS("LazyObjectProperty", "UObjectProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("SoftClassProperty", "USoftObjectProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("SoftObjectProperty", "UObjectProperty")
}

}  // namespace pyunrealsdk::unreal
#endif
