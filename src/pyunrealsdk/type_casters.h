#ifndef PYUNREALSDK_TYPE_CASTERS_H
#define PYUNREALSDK_TYPE_CASTERS_H

// This file is included in the pch, so we can't include it here, just need to assume we have access
// to everything in the standard library which we need already

// Still need to include fname however, so we can create new ones
#include "unrealsdk/unreal/structs/fname.h"

namespace unrealsdk::unreal {

class UObject;

}

namespace pyunrealsdk::type_casters {

/**
 * @brief Downcaster for objects inheriting from UObject.
 *
 * @param src The source object to try downcast.
 * @param type The downcast type to use.
 * @return The source object.
 */
const void* downcast_unreal(const unrealsdk::unreal::UObject* src, const std::type_info*& type);

}  // namespace pyunrealsdk::type_casters

namespace pybind11 {

// Make the UObject hierarchy automatically downcast
template <typename itype>
struct polymorphic_type_hook<
    itype,
    detail::enable_if_t<std::is_base_of<unrealsdk::unreal::UObject, itype>::value>> {
    static const void* get(const unrealsdk::unreal::UObject* src, const std::type_info*& type) {
        return pyunrealsdk::type_casters::downcast_unreal(src, type);
    }
};

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
