#ifndef PYUNREALSDK_BASE_BINDINGS_H
#define PYUNREALSDK_BASE_BINDINGS_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace unrealsdk::unreal {

class UClass;
class UScriptStruct;

}  // namespace unrealsdk::unreal

namespace pyunrealsdk {

/**
 * @brief Registers all bindings in the base unrealsdk module.
 *
 * @param mod The module to register within.
 */
void register_base_bindings(py::module_& mod);

/**
 * @brief Given an argument which accepts either a class or it's name, attempt to find the class.
 * @note Throws if unable to find a valid object.
 * @note Does not allow null pointers.
 *
 * @param cls_arg The class argument.
 * @param fully_qualified If the name is fully qualified, or nullopt to autodetect.
 * @return The class object.
 */
unrealsdk::unreal::UClass* evaluate_class_arg(
    const std::variant<unrealsdk::unreal::UClass*, std::wstring>& cls_arg,
    std::optional<bool> fully_qualified);

/**
 * @brief Given an argument which accepts either a scriptstruct or it's name, attempt to find it.
 * @note Throws if unable to find a valid object.
 * @note Does not allow null pointers.
 *
 * @param struct_arg The struct argument.
 * @param fully_qualified If the name is fully qualified, or nullopt to autodetect.
 * @return The struct object.
 */
unrealsdk::unreal::UScriptStruct* evaluate_scriptstruct_arg(
    const std::variant<unrealsdk::unreal::UScriptStruct*, std::wstring>& struct_arg,
    std::optional<bool> fully_qualified);

}  // namespace pyunrealsdk

#endif

#endif /* PYUNREALSDK_BASE_BINDINGS_H */
