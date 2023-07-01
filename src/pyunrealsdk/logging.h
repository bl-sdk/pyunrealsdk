#ifndef PYUNREALSDK_LOGGING_H
#define PYUNREALSDK_LOGGING_H

#include "pyunrealsdk/pch.h"

namespace pyunrealsdk::logging {

/**
 * @brief Registers everything needed for the logging module.
 *
 * @param module The module to register within.
 */
void register_module(py::module_& module);

/**
 * @brief Performs python initialization of the logging module.
 * @note Sets up the stdout/err redirection.
 */
void py_init(void);

}  // namespace pyunrealsdk::logging

#endif /* PYUNREALSDK_LOGGING_H */
