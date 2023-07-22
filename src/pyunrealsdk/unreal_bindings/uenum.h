#ifndef PYUNREALSDK_UNREAL_BINDINGS_UENUM_H
#define PYUNREALSDK_UNREAL_BINDINGS_UENUM_H

#include "pyunrealsdk/pch.h"

namespace unrealsdk::unreal {

class UEnum;

}

namespace pyunrealsdk::unreal {

/**
 * @brief Registers UEnum.
 *
 * @param mod The module to register within.
 */
void register_uenum(py::module_& mod);

/**
 * @brief Creates a python enum from an unreal enum.
 *
 * @param enum_obj The unreal enum object.
 * @return The python enum.
 */
py::object enum_as_py_enum(const unrealsdk::unreal::UEnum* enum_obj);

}  // namespace pyunrealsdk::unreal

#endif /* PYUNREALSDK_UNREAL_BINDINGS_UENUM_H */
