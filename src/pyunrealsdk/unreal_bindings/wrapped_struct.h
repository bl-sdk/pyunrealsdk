#ifndef PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_STRUCT_H
#define PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_STRUCT_H

#include "pyunrealsdk/pch.h"

namespace pyunrealsdk::unreal {

/**
 * @brief Registers WrappedStruct.
 *
 * @param mod The module to register within.
 */
void register_wrapped_struct(py::module_& mod);

}  // namespace pyunrealsdk::unreal

#endif /* PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_STRUCT_H */
