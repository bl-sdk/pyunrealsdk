#ifndef PYUNREALSDK_UNREAL_BINDINGS_UCLASS_H
#define PYUNREALSDK_UNREAL_BINDINGS_UCLASS_H

namespace pyunrealsdk::unreal {

/**
 * @brief Registers all classes which derive from UObject.
 *
 * @param module The module to register within.
 */
void register_uobject_children(py::module_& module);

}  // namespace pyunrealsdk::unreal

#endif /* PYUNREALSDK_UNREAL_BINDINGS_UCLASS_H */
