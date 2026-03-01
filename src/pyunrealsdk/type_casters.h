#ifndef PYUNREALSDK_TYPE_CASTERS_H
#define PYUNREALSDK_TYPE_CASTERS_H

// This file is included in the pch, so we can't include it here, just need to assume we have access
// to everything in the standard library which we need already

// Still need to include fname however, so we can create new ones
#include "unrealsdk/unreal/structs/fname.h"

namespace unrealsdk::unreal {

class UObject;
class WrappedStruct;
struct FField;

}  // namespace unrealsdk::unreal

namespace pyunrealsdk::type_casters {

#ifdef PYUNREALSDK_INTERNAL

/**
 * @brief Downcaster for objects inheriting from UObject.
 *
 * @param src The source object to try downcast.
 * @param type The downcast type to use.
 * @return The source object.
 */
const void* downcast_unreal(const unrealsdk::unreal::UObject* src, const std::type_info*& type);
const void* downcast_unreal(const unrealsdk::unreal::FField* src, const std::type_info*& type);

#endif

#ifndef PYUNREALSDK_INTERNAL
// These casters are only provided for external use, when working internally use regular `py::cast`.

/**
 * @brief Casts an unreal object to a python object.
 *
 * @param src The source unreal object.
 * @return The python object.
 */
py::object cast(unrealsdk::unreal::UObject* src);
py::object cast(unrealsdk::unreal::FField* src);
py::object cast(unrealsdk::unreal::WrappedStruct* src);

/**
 * @brief Casts a python object to an unreal object.
 *
 * @tparam T The unreal type to cast to.
 * @param src The source python object.
 * @return The unreal object.
 */
template <typename T>
T cast(const py::object& src);

template <>
unrealsdk::unreal::UObject* cast<unrealsdk::unreal::UObject*>(const py::object& src);
template <>
unrealsdk::unreal::FField* cast<unrealsdk::unreal::FField*>(const py::object& src);

#endif

}  // namespace pyunrealsdk::type_casters

namespace pybind11 {

#ifdef PYUNREALSDK_INTERNAL
// Make the UObject/FField hierarchies automatically downcast

template <typename itype>
struct polymorphic_type_hook<
    itype,
    detail::enable_if_t<std::is_base_of_v<unrealsdk::unreal::UObject, itype>>> {
    static const void* get(const unrealsdk::unreal::UObject* src, const std::type_info*& type) {
        return pyunrealsdk::type_casters::downcast_unreal(src, type);
    }
};

template <typename itype>
struct polymorphic_type_hook<
    itype,
    detail::enable_if_t<std::is_base_of_v<unrealsdk::unreal::FField, itype>>> {
    static const void* get(const unrealsdk::unreal::FField* src, const std::type_info*& type) {
        return pyunrealsdk::type_casters::downcast_unreal(src, type);
    }
};

#endif

namespace detail {

// Allow FNames to cast to/from python strings
template <>
struct type_caster<unrealsdk::unreal::FName> {
   public:
    // The name we set here is used in the signatures added to the docstring, so set it as `str` so
    // they appear valid
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory, readability-identifier-length)
    PYBIND11_TYPE_CASTER(unrealsdk::unreal::FName, py::detail::const_name("str"));

    bool load(handle src, bool /*convert*/) {
        py::ssize_t size = 0;
        const char* str = PyUnicode_AsUTF8AndSize(src.ptr(), &size);

        if (str == nullptr) {
            return false;
        }

        value = unrealsdk::unreal::FName(std::string{str, static_cast<size_t>(size)});
        return true;
    }

    static handle cast(unrealsdk::unreal::FName src,
                       return_value_policy /* policy */,
                       handle /* parent */) {
        std::string name = src;
        return PyUnicode_FromStringAndSize(name.c_str(), static_cast<py::ssize_t>(name.size()));
    }
};

}  // namespace detail

}  // namespace pybind11

#endif /* PYUNREALSDK_TYPE_CASTERS_H */
