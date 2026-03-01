#ifndef PYUNREALSDK_UNREAL_BINDINGS_GBXDEFPTR_H
#define PYUNREALSDK_UNREAL_BINDINGS_GBXDEFPTR_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace pyunrealsdk::unreal {

/**
 * @brief Registers GbxDefPtr.
 *
 * @param mod The module to register within.
 */
void register_gbxdefptr(py::module_& mod);

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_GBXDEFPTR_H */
