#ifndef PYUNREALSDK_COMMANDS_H
#define PYUNREALSDK_COMMANDS_H

#include "pyunrealsdk/pch.h"

namespace pyunrealsdk::commands {

/**
 * @brief Registers everything needed for the commands module.
 *
 * @param mod The module to register within.
 */
void register_module(py::module_& mod);

/**
 * @brief Registers all our custom console commands.
 */
void register_commands(void);

}  // namespace pyunrealsdk::commands

#endif /* PYUNREALSDK_COMMANDS_H */
