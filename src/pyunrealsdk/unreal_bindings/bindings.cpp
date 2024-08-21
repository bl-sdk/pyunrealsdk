#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/bound_function.h"
#include "pyunrealsdk/unreal_bindings/persistent_object_ptr_property.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "pyunrealsdk/unreal_bindings/uenum.h"
#include "pyunrealsdk/unreal_bindings/uobject.h"
#include "pyunrealsdk/unreal_bindings/uobject_children.h"
#include "pyunrealsdk/unreal_bindings/weak_pointer.h"
#include "pyunrealsdk/unreal_bindings/wrapped_array.h"
#include "pyunrealsdk/unreal_bindings/wrapped_struct.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/classes/ustruct_funcs.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_module(py::module_& mod) {
    auto unreal = mod.def_submodule("unreal");

    register_property_helpers(unreal);

    register_uobject(unreal);
    register_uobject_children(unreal);
    register_uenum(unreal);
    register_wrapped_array(unreal);
    register_wrapped_struct(unreal);
    register_bound_function(unreal);
    register_weak_pointer(unreal);
    register_persistent_object_properties(unreal);
}

}  // namespace pyunrealsdk::unreal

#endif
