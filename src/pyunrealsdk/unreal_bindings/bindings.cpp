#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "pyunrealsdk/unreal_bindings/uobject.h"
#include "pyunrealsdk/unreal_bindings/uobject_children.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/classes/ustruct_funcs.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_module(py::module_& mod) {
    auto unreal = mod.def_submodule("unreal");

    register_uobject(unreal);
    register_uobject_children(unreal);
    register_property_helpers(unreal);
}

}  // namespace pyunrealsdk::unreal
