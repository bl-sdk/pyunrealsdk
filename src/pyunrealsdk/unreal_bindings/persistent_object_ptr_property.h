#ifndef PYUNREALSDK_UNREAL_BINDINGS_PERSISTENT_OBJECT_PTR_PROPERTY_H
#define PYUNREALSDK_UNREAL_BINDINGS_PERSISTENT_OBJECT_PTR_PROPERTY_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace pyunrealsdk::unreal {

/**
 * @brief Registers the persistent object properties.
 *
 * @param module The module to register within.
 */
void register_persistent_object_properties(py::module_& mod);

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_PERSISTENT_OBJECT_PTR_PROPERTY_H */
