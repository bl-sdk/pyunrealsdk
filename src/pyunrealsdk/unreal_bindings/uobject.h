#ifndef PYUNREALSDK_UNREAL_BINDINGS_UOBJECT_H
#define PYUNREALSDK_UNREAL_BINDINGS_UOBJECT_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace pyunrealsdk::unreal {

/**
 * @brief Registers UObject.
 *
 * @param mod The module to register within.
 */
void register_uobject(py::module_& mod);

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_UOBJECT_H */
