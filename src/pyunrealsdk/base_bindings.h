#ifndef PYUNREALSDK_BASE_BINDINGS_H
#define PYUNREALSDK_BASE_BINDINGS_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace pyunrealsdk {

/**
 * @brief Registers all bindings in the base unrealsdk module.
 *
 * @param mod The module to register within.
 */
void register_base_bindings(py::module_& mod);

}  // namespace pyunrealsdk

#endif

#endif /* PYUNREALSDK_BASE_BINDINGS_H */
