#ifndef PYUNREALSDK_ENV_H
#define PYUNREALSDK_ENV_H

#include "pyunrealsdk/pch.h"
#include "unrealsdk/env.h"

namespace pyunrealsdk::env {

using unrealsdk::env::defined;
using unrealsdk::env::env_var_key;
using unrealsdk::env::get;
using unrealsdk::env::get_numeric;

const constexpr env_var_key INIT_SCRIPT = "PYUNREALSDK_INIT_SCRIPT";
const constexpr env_var_key DEBUGPY = "PYUNREALSDK_DEBUGPY";
const constexpr env_var_key PYEXEC_ROOT = "PYUNREALSDK_PYEXEC_ROOT";

namespace defaults {

const constexpr auto INIT_SCRIPT = "__main__.py";
// DEBUGPY - defaults to empty string (only used in defined checks)
// PYEXEC_ROOT - defaults to empty string (cwd)

}  // namespace defaults

}  // namespace pyunrealsdk::env

#endif /* PYUNREALSDK_ENV_H */
