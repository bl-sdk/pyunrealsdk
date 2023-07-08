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
 * @tparam T The wrapped unreal object type
 * @tparam Options Extra options for the class definition.
 */
template <typename T, typename... Options>
using PyUEClass = py::class_<T, Options..., std::unique_ptr<T, py::nodelete>>;

}  // namespace pyunrealsdk::unreal

#endif /* PYUNREALSDK_UNREAL_BINDINGS_UNREAL_H */
