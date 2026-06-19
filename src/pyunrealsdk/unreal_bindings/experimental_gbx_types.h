#ifndef PYUNREALSDK_UNREAL_BINDINGS_EXPERIMENTAL_GBX_TYPES_H
#define PYUNREALSDK_UNREAL_BINDINGS_EXPERIMENTAL_GBX_TYPES_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace pyunrealsdk::unreal {

/**
 * @brief Registers the experimental types for gearbox properties.
 *
 * @param mod The module to register within.
 */
void register_experimental_gbx_types(py::module_& mod);

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_EXPERIMENTAL_GBX_TYPES_H */
