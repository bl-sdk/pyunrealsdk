#ifndef PYUNREALSDK_UNREAL_BINDINGS_UOBJECT_H
#define PYUNREALSDK_UNREAL_BINDINGS_UOBJECT_H

namespace pyunrealsdk::unreal {

/**
 * @brief Registers UObject.
 *
 * @param module The module to register within.
 */
void register_uobject(py::module_& module);

}  // namespace pyunrealsdk::unreal

#endif /* PYUNREALSDK_UNREAL_BINDINGS_UOBJECT_H */
