#ifndef PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_MULTICAST_DELEGATE_H
#define PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_MULTICAST_DELEGATE_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace pyunrealsdk::unreal {

/**
 * @brief Registers WrappedMulticastDelegate.
 *
 * @param module The module to register within.
 */
void register_wrapped_multicast_delegate(py::module_& mod);

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_MULTICAST_DELEGATE_H */
