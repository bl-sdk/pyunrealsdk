#ifndef PYUNREALSDK_UNREAL_BINDINGS_WEAK_POINTER_H
#define PYUNREALSDK_UNREAL_BINDINGS_WEAK_POINTER_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace pyunrealsdk::unreal {

/**
 * @brief Registers WeakPointer.
 *
 * @param module The module to register within.
 */
void register_weak_pointer(py::module_& mod);

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_WEAK_POINTER_H */
