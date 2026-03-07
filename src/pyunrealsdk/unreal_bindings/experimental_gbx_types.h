#ifndef PYUNREALSDK_UNREAL_BINDINGS_EXPERIMENTAL_GBX_TYPES_H
#define PYUNREALSDK_UNREAL_BINDINGS_EXPERIMENTAL_GBX_TYPES_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace unrealsdk::unreal {

struct FGbxInlineStruct;
class ZGbxInlineStructProperty;

}  // namespace unrealsdk::unreal

namespace pyunrealsdk::unreal {

/**
 * @brief Registers the experimental types for gearbox properties.
 *
 * @param mod The module to register within.
 */
void register_experimental_gbx_types(py::module_& mod);

/**
 * @brief Converts a ZGbxInlineStructProperty to our experimental internal representation.
 *
 * @param prop The property to convert.
 * @param inline_struct The read off unreal value.
 * @return The value as a python object.
 */
py::object convert_gbx_inline_struct_prop(const unrealsdk::unreal::ZGbxInlineStructProperty* prop,
                                          unrealsdk::unreal::FGbxInlineStruct* inline_struct);

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_EXPERIMENTAL_GBX_TYPES_H */
