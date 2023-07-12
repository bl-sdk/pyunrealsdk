#ifndef PYUNREALSDK_HOOKS_H
#define PYUNREALSDK_HOOKS_H

#include "pyunrealsdk/pch.h"

namespace pyunrealsdk::hooks {

/**
 * @brief Registers everything needed for the hooks module.
 *
 * @param mod The module to register within.
 */
void register_module(py::module_& mod);

}  // namespace pyunrealsdk::hooks

#endif /* PYUNREALSDK_HOOKS_H */
