#ifndef PYUNREALSDK_HOOKS_H
#define PYUNREALSDK_HOOKS_H

#include "pyunrealsdk/pch.h"

namespace pyunrealsdk::hooks {

#ifdef PYUNREALSDK_INTERNAL
/**
 * @brief Registers everything needed for the hooks module.
 *
 * @param mod The module to register within.
 */
void register_module(py::module_& mod);

/**
 * @brief Checks if we should automatically call inject next call for bound function calls.
 *
 * @return True if we should auto inject python calls.
 */
bool should_auto_inject_py_calls(void);
#endif

/**
 * @brief Checks if a python object is the block sentinel.
 *
 * @param obj The object to check.
 * @return True if the object says to block execution.
 */
bool is_block_sentinel(const py::object& obj);

}  // namespace pyunrealsdk::hooks

#endif /* PYUNREALSDK_HOOKS_H */
