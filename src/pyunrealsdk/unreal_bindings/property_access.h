#ifndef PYUNREALSDK_UNREAL_BINDINGS_PROPERTY_ACCESS_H
#define PYUNREALSDK_UNREAL_BINDINGS_PROPERTY_ACCESS_H

#include "pyunrealsdk/pch.h"
#include "unrealsdk/unreal/structs/tfieldvariant.h"

#ifdef PYUNREALSDK_INTERNAL

namespace unrealsdk::unreal {

struct FName;
struct FField;
class UObject;
class UField;
class UStruct;
class ZProperty;

template <typename T>
class UnrealPointer;

}  // namespace unrealsdk::unreal

namespace pyunrealsdk::unreal {

/**
 * @brief Registers the helpers functions for property accesses.
 *
 * @param mod The module to register within.
 */
void register_property_helpers(py::module_& mod);

namespace {
#if UNREALSDK_PROPERTIES_ARE_FFIELD
using base_variant =
    unrealsdk::unreal::TFieldVariant<unrealsdk::unreal::ZProperty, unrealsdk::unreal::UField>;
#else
using base_variant = unrealsdk::unreal::TFieldVariantStub<unrealsdk::unreal::UField>;
#endif
}  // namespace

struct PyFieldVariant : public base_variant {
    PyFieldVariant(void) = default;
    PyFieldVariant(std::nullptr_t) {};
#if UNREALSDK_PROPERTIES_ARE_FFIELD
    PyFieldVariant(const unrealsdk::unreal::ZProperty* field) : base_variant(field) {}
#endif
    PyFieldVariant(const unrealsdk::unreal::UField* obj) : base_variant(obj) {};
    PyFieldVariant(const base_variant& other) : base_variant(other) {}
    PyFieldVariant(base_variant&& other) noexcept : base_variant(std::move(other)) {}
    PyFieldVariant(const PyFieldVariant& other) = default;
    PyFieldVariant(PyFieldVariant&& other) noexcept = default;

    PyFieldVariant& operator=(const PyFieldVariant& other) = default;
    PyFieldVariant& operator=(PyFieldVariant&& other) noexcept = default;

    ~PyFieldVariant(void) = default;

#if UNREALSDK_PROPERTIES_ARE_FFIELD
    // A type guaranteed to be convertible to this from python.
    using from_py_type =
        std::variant<std::nullptr_t, unrealsdk::unreal::ZProperty*, unrealsdk::unreal::UField*>;

    PyFieldVariant(const from_py_type& var);
#else
    // A type guaranteed to be convertible to this from python.
    using from_py_type = unrealsdk::unreal::UField*;
#endif

    /**
     * @brief Gets the contained field, if it holds a property.
     *
     * @return The contained property, or nullptr if the wrong type.
     */
    [[nodiscard]] unrealsdk::unreal::ZProperty* as_prop(void) const;

    /**
     * @brief Gets the contained field, if it holds a UField which *is not* a ZProperty.
     *
     * @return The contained property, or nullptr if the wrong type.
     */
    [[nodiscard]] unrealsdk::unreal::UField* as_non_prop_field(void) const;
};

/**
 * @brief Searches for a field on a struct, throwing an attribute error if it doesn't exist.
 *
 * @param name The field name.
 * @param type The type of the unreal object this access is reading off of.
 * @return The field. Invalid keys throw, so will never be null.
 */
PyFieldVariant py_find_field(const unrealsdk::unreal::FName& name,
                             const unrealsdk::unreal::UStruct* type);

/**
 * @brief Implements `__dir__`.
 *
 * @param self The base object - used to get the non-dynamic fields.
 * @param type The type of the object.
 * @return A list of all attributes on the object.
 */
std::vector<std::string> py_dir(const py::object& self, const unrealsdk::unreal::UStruct* type);

/**
 * @brief Implements `__getattr__`.
 *
 * @param field The field to get.
 * @param base_addr The base address of the object.
 * @param parent Pointer to a parent allocation to copy ownership from.
 * @param func_obj The object to bind functions to. If nullptr, does not allow getting functions.
 * @return The retrieved value.
 */
py::object py_getattr(PyFieldVariant field,
                      uintptr_t base_addr,
                      const unrealsdk::unreal::UnrealPointer<void>& parent,
                      unrealsdk::unreal::UObject* func_obj = nullptr);

/**
 * @brief Sets an unreal field to a python object directly.
 * @note This is not suitable as an implementation of `__setattr__`, as it bypasses any other Python
 *       field setting logic (e.g. properties). Getattr is only called on failed gets, while
 *       setattr is called on *all* sets. To make it symmetric, try call the super setattr first,
 *       and only call this if it fails.
 *
 * @param field The field to get.
 * @param base_addr The base address of the object.
 * @param value The value to set.
 */
void py_setattr_direct(PyFieldVariant field, uintptr_t base_addr, const py::object& value);

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_PROPERTY_ACCESS_H */
