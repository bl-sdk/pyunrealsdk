#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
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
}

UField* get_field_from_py_key(const py::object& key, UStruct* type) {
    if (py::isinstance<py::str>(key)) {
        std::string key_str = py::str(key);
        try {
            return type->find(key_str);
        } catch (const std::invalid_argument&) {
            throw py::attribute_error(
                unrealsdk::fmt::format("'{}' object has no attribute '{}'", type->Name, key_str));
        }
    }

    if (py::isinstance<UField>(key)) {
        auto field = py::cast<UField*>(key);
        if (field == nullptr) {
            throw py::attribute_error("cannot access null attribute");
        }
        return field;
    }

    std::string key_type_name = py::str(py::type::of(key).attr("__name__"));
    throw py::attribute_error(
        unrealsdk::fmt::format("attribute key has unknown type '{}'", key_type_name));
}

}  // namespace pyunrealsdk::unreal
