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
 * @param out_struct The existing struct to write into.
 * @param args The python args.
 * @param kwargs The python kwargs.
 * @return The new wrapped struct.
 */
unrealsdk::unreal::WrappedStruct make_struct(
    std::variant<const unrealsdk::unreal::UFunction*, const unrealsdk::unreal::UScriptStruct*> type,
    const py::args& args,
    const py::kwargs& kwargs);
void make_struct(unrealsdk::unreal::WrappedStruct& out_struct,
                 const py::args& args,
                 const py::kwargs& kwargs);

/**
 * @brief Checks if a python object is the ignore struct sentinel.
 *
 * @param obj The object to check.
 * @return True if the object is the ignore struct sentinel.
 */
bool is_ignore_struct_sentinel(const py::object& obj);

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_STRUCT_H */
