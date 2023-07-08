#ifndef PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_ARRAY_H
#define PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_ARRAY_H

namespace pyunrealsdk::unreal {

/**
 * @brief Registers WrappedArray.
 *
 * @param mod The module to register within.
 */
void register_wrapped_array(py::module_& mod);

}  // namespace pyunrealsdk::unreal

#endif /* PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_ARRAY_H */
