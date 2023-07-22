#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/uenum.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/classes/ufield.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

py::object enum_as_py_enum(const UEnum* enum_obj) {
    // Use IntFlag, as it natively supports unknown values
    static auto intenum = py::module_::import("enum").attr("IntFlag");
    static std::unordered_map<const UEnum*, py::object> enum_cache{};

    if (!enum_cache.contains(enum_obj)) {
        enum_cache.emplace(enum_obj, intenum(enum_obj->Name, enum_obj->get_names()));
    }

    return enum_cache[enum_obj];
}

void register_uenum(py::module_& mod) {
    PyUEClass<UEnum, UField>(mod, "UEnum")
        .def("_as_py", &enum_as_py_enum,
             "Generates a compatible IntFlag enum.\n"
             "\n"
             "Returns:\n"
             "    An IntFlag enum compatible with this enum.");
}

}  // namespace pyunrealsdk::unreal
