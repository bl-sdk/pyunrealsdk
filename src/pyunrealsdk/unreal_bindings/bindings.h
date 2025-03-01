#ifndef PYUNREALSDK_UNREAL_BINDINGS_UNREAL_H
#define PYUNREALSDK_UNREAL_BINDINGS_UNREAL_H

#include "pyunrealsdk/pch.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ustruct.h"

#ifdef PYUNREALSDK_INTERNAL

namespace pyunrealsdk::unreal {

/**
 * @brief Registers everything needed for the unreal module.
 *
 * @param mod The module to register within.
 */
void register_module(py::module_& mod);

/**
 * @brief Helper class for mapping unreal objects to python.
 *
 * @tparam T The wrapped unreal object type
 * @tparam Options Extra options for the class definition.
 */
template <typename T, typename... Options>
class PY_OBJECT_VISIBILITY PyUEClass
    : public py::class_<T, Options..., std::unique_ptr<T, py::nodelete>> {
   public:
    using py::class_<T, Options..., std::unique_ptr<T, py::nodelete>>::class_;

    /**
     * @brief Helper type to define one of our C++ "properties" as a python property.
     * @note These must be first calls, as the standard def functions return a base py::class_,
     *       rendering this function inaccessible.
     *
     * @tparam C The type of this class, should be picked up automatically.
     * @tparam D The type of the property, should be picked up automatically.
     * @tparam Extra Any extra template args to forward.
     * @param name The name of the property.
     * @param getter The getter function. Needs to be specialized, should be non-cost.
     * @param extra Any extra args to forward.
     * @return A reference to the same class object.
     */
    template <typename C, typename D, typename... Extra>
    PyUEClass& def_member_prop(const char* name, D& (*getter)(C&), const Extra&... extra) {
        this->def_property(
            name, [getter](C& self) { return getter(self); },
            [getter](C& self, D&& value) { getter(self) = std::move(value); }, extra...);
        return *this;
    }
};

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_UNREAL_H */
