#ifndef PYUNREALSDK_UNREAL_BINDINGS_UNREAL_H
#define PYUNREALSDK_UNREAL_BINDINGS_UNREAL_H

#include "pyunrealsdk/pch.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ustruct.h"

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
 * @tparam T The wrapped unreall object type
 * @tparam Options Extra options for the class definition.
 */
template <typename T, typename... Options>
class PyUEClass : public py::class_<T, Options..., std::unique_ptr<T, py::nodelete>> {
   public:
    using py::class_<T, Options..., std::unique_ptr<T, py::nodelete>>::class_;

    /**
     * @brief Defines new and init as immediately throwing, to prevent python code from constructing
     *        objects of this type.
     *
     * @return A reference to this class.
     */
    PyUEClass& def_no_constructor(void) {
        this->def("__new__",
                  [](const py::args&, const py::kwargs&) {
                      throw py::type_error("Cannot create new instances of unreal objects.");
                  })
            .def("__init__", [](const py::args&, const py::kwargs&) {
                throw py::type_error("Cannot create new instances of unreal objects.");
            });
        return *this;
    }

    /**
     * @brief Defines a get address function.
     *
     * @return A reference to this class.
     */
    PyUEClass& def_get_address(void) {
        this->def(
            "_get_address", [](T* self) { return reinterpret_cast<uintptr_t>(self); },
            "Gets the address of this object, for debugging.\n"
            "\n",
            "Returns:\n"
            "    This object's address.");
        return *this;
    }
};

}  // namespace pyunrealsdk::unreal

#endif /* PYUNREALSDK_UNREAL_BINDINGS_UNREAL_H */
