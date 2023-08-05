#ifndef PYUNREALSDK_UNREAL_BINDINGS_BOUND_FUNCTION_H
#define PYUNREALSDK_UNREAL_BINDINGS_BOUND_FUNCTION_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace pyunrealsdk::unreal {

/**
 * @brief Registers BoundFunction.
 *
 * @param module The module to register within.
 */
void register_bound_function(py::module_& mod);

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_BOUND_FUNCTION_H */
