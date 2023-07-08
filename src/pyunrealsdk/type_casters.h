#ifndef PYUNREALSDK_TYPE_CASTERS_H
#define PYUNREALSDK_TYPE_CASTERS_H

#include "unrealsdk/unreal/structs/fname.h"

namespace pybind11::detail {

template <>
struct type_caster<unrealsdk::unreal::FName> {
   public:
    PYBIND11_TYPE_CASTER(unrealsdk::unreal::FName, py::detail::const_name("FName"));

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
}  // namespace pybind11::detail

#endif /* PYUNREALSDK_TYPE_CASTERS_H */
