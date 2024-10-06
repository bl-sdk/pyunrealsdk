#ifndef PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_STRUCT_H
#define PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_STRUCT_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace unrealsdk::unreal {

class UFunction;
class UScriptStruct;
class WrappedStruct;

}  // namespace unrealsdk::unreal

namespace pyunrealsdk::unreal {

/**
 * @brief Registers WrappedStruct.
 *
 * @param mod The module to register within.
 */
void register_wrapped_struct(py::module_& mod);

/**
 * @brief Creates a new wrapped struct using python args.
 *
 * @param type The type of struct to make.
 * @param args The python args.
 * @param kwargs The python kwargs.
 * @return The new wrapped struct.
 */
unrealsdk::unreal::WrappedStruct make_struct(
    std::variant<const unrealsdk::unreal::UFunction*, const unrealsdk::unreal::UScriptStruct*> type,
    const py::args& args,
    const py::kwargs& kwargs);

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_STRUCT_H */
