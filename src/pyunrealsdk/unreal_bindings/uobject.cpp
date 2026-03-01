#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/uobject.h"
#include "pyunrealsdk/static_py_object.h"
#include "pyunrealsdk/stubgen.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/properties/zproperty.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

namespace {

// This needs to be a function to avoid an unreachable code warning in `pybind11/detail/init.h` in
// `no_nullptr` when compiling with MSVC.
// I assume this is something to do with inlining optimizations - since we don't actually return, it
// can assume it's nullptr, somehow ignoring the throw here, but not the one later?
// Not thrilled with this as a solution, but it works for now

// __init__
UObject* uobject_init(const py::args& /* args */, const py::kwargs& /* kwargs */) {
    throw py::type_error("Cannot create new instances of unreal objects.");
}

// Dummy class to make the context manager on
struct ContextManager {};

size_t should_notify_counter = 0;

}  // namespace

void register_uobject(py::module_& mod) {
    PYUNREALSDK_STUBGEN_MODULE_N("unrealsdk.unreal")

    PyUEClass<UObject>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UObject", ),
        PYUNREALSDK_STUBGEN_DOCSTRING(
            "The base class of all unreal objects.\n"
            "\n"
            "Most objects you interact with will be this type in python, even if their unreal\n"
            "class is something different.\n"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ObjectFlags", "int"), &UObject::ObjectFlags)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("InternalIndex", "int"), &UObject::InternalIndex)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Class", "UClass"), &UObject::Class)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Name", "str"), &UObject::Name)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Outer", "UObject"), &UObject::Outer)
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD("__new__"),
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of unreal objects.");
             })
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD_N("__init__") py::init(&uobject_init))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__repr__", "str"),
            [](UObject* self) {
                return std::format("{}'{}'", self->Class()->Name(),
                                   unrealsdk::utils::narrow(self->get_path_name()));
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets this object's full name.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    This object's name.\n"))
        .def(PYUNREALSDK_STUBGEN_METHOD("_path_name", "str"), &UObject::get_path_name,
             PYUNREALSDK_STUBGEN_DOCSTRING("Gets this object's path name, excluding the class.\n"
                                           "\n"
                                           "Returns:\n"
                                           "    This object's name.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__dir__", "list[str]"),
            [](const py::object& self) { return py_dir(self, py::cast<UObject*>(self)->Class()); },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Gets the attributes which exist on this object.\n"
                "\n"
                "Includes both python attributes and unreal fields. This can be changed to only\n"
                "python attributes by calling dir_includes_unreal.\n"
                "\n"
                "Returns:\n"
                "    A list of attributes which exist on this object.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__getattr__", "Any"),
            [](UObject* self, const FName& name) {
                return py_getattr(py_find_field(name, self->Class()),
                                  reinterpret_cast<uintptr_t>(self), nullptr, self);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Reads an unreal field off of the object.\n"
                                          "\n"
                                          "Automatically looks up the relevant UField.\n"
                                          "\n"
                                          "Args:\n"
                                          "    name: The name of the field to get.\n"
                                          "Returns:\n"
                                          "    The field's value.\n"),
            PYUNREALSDK_STUBGEN_ARG("name"_a, "str", ))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_get_field", "Any"),
            [](UObject* self, PyFieldVariant::from_py_type field) {
                const PyFieldVariant var{field};
                if (var == nullptr) {
                    throw py::attribute_error("cannot access null attribute");
                }
                return py_getattr(var, reinterpret_cast<uintptr_t>(self), nullptr, self);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Reads an unreal field off of the object.\n"
                "\n"
                "In performance critical situations, rather than use __getattr__, you can look up\n"
                "the UField beforehand (via obj.Class._find()), then pass it directly to this\n"
                "function. This does not get validated, passing a field which doesn't exist on\n"
                "the object is undefined behaviour.\n"
                "\n"
                "Args:\n"
                "    field: The field to get.\n"
                "Returns:\n"
                "    The field's value.\n"),
#if UNREALSDK_PROPERTIES_ARE_FFIELD
            PYUNREALSDK_STUBGEN_ARG("field"_a, "UField | ZProperty", )
#else
            PYUNREALSDK_STUBGEN_ARG("field"_a, "UField", )
#endif
                )
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__setattr__", "None"),
            [](UObject* self, const py::str& name, const py::object& value) {
                // See if the standard setattr would work first, in case we're being called on an
                // existing field. Getattr is only called on failure, but setattr is always called.
                PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> storage;
                auto& setattr = storage
                                    .call_once_and_store_result([]() {
                                        return py::module::import("builtins")
                                            .attr("object")
                                            .attr("__setattr__");
                                    })
                                    .get_stored();

                try {
                    setattr(self, name, value);
                    return;
                } catch (py::error_already_set& e) {
                    if (!e.matches(PyExc_AttributeError)) {
                        throw;
                    }
                }

                auto field = py_find_field(py::cast<FName>(name), self->Class());
                py_setattr_direct(field, reinterpret_cast<uintptr_t>(self), value);

                if (should_notify_counter > 0) {
                    auto prop = field.as_prop();
                    if (prop != nullptr) {
                        self->post_edit_change_property(prop);
                    }
                }
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Writes a value to an unreal field on the object.\n"
                                          "\n"
                                          "Automatically looks up the relevant UField.\n"
                                          "\n"
                                          "Args:\n"
                                          "    name: The name of the field to set.\n"
                                          "    value: The value to write.\n"),
            PYUNREALSDK_STUBGEN_ARG("name"_a, "str", ), PYUNREALSDK_STUBGEN_ARG("value"_a, "Any", ))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_set_field", "None"),
            [](UObject* self, PyFieldVariant::from_py_type field, const py::object& value) {
                const PyFieldVariant var{field};
                py_setattr_direct(var, reinterpret_cast<uintptr_t>(self), value);

                if (should_notify_counter > 0) {
                    auto prop = var.as_prop();
                    if (prop != nullptr) {
                        self->post_edit_change_property(prop);
                    }
                }
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Writes a value to an unreal field on the object.\n"
                "\n"
                "In performance critical situations, rather than use __setattr__, you can look up\n"
                "the UField beforehand (via obj.Class._find()), then pass it directly to this\n"
                "function. This does not get validated, passing a field which doesn't exist on\n"
                "the object is undefined behaviour.\n"
                "\n"
                "Args:\n"
                "    field: The field to set.\n"
                "    value: The value to write.\n"),
#if UNREALSDK_PROPERTIES_ARE_FFIELD
            PYUNREALSDK_STUBGEN_ARG("field"_a, "UField | ZProperty", ),
#else
            PYUNREALSDK_STUBGEN_ARG("field"_a, "UField", ),
#endif
            PYUNREALSDK_STUBGEN_ARG("value"_a, "Any", ))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_get_address", "int"),
            [](UObject* self) { return reinterpret_cast<uintptr_t>(self); },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets the address of this object, for debugging.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    This object's address.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_post_edit_change_property", "None"),
            [](UObject* self, std::variant<FName, ZProperty*> prop) {
                std::visit([self](auto&& val) { self->post_edit_change_property(val); }, prop);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Notifies the engine that we've made an external change to a property.\n"
                "\n"
                "This only works on top level properties, those directly on the object.\n"
                "\n"
                "Also see the notify_changes() context manager, which calls this automatically.\n"
                "\n"
                "Args:\n"
                "    prop: The property, or the name of the property, which was changed.\n"),
            PYUNREALSDK_STUBGEN_ARG("prop"_a, "ZProperty | str", ))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_post_edit_change_chain_property", "None"),
            [](UObject* self, ZProperty* prop, const py::args& args) {
                std::vector<ZProperty*> chain;
                chain.reserve(args.size());

                for (const auto& val : args) {
                    chain.push_back(py::cast<ZProperty*>(val));
                }
                self->post_edit_change_chain_property(prop, chain);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Notifies the engine that we've made an external change to a chain of properties.\n"
                "\n"
                "This version allows notifying about changes inside (nested) structs.\n"
                "\n"
                "Args:\n"
                "    prop: The property which was changed.\n"
                "    *chain: The chain of properties to follow.\n"),
            PYUNREALSDK_STUBGEN_ARG("prop"_a, "ZProperty", )
                PYUNREALSDK_STUBGEN_ARG_N("*chain"_a, "ZProperty", ));

    // Create under an empty handle to prevent this type being normally accessible
    py::classh<ContextManager>(py::handle(), "context_manager", pybind11::module_local())
        .def("__enter__", [](const py::object& /*self*/) { should_notify_counter++; })
        .def("__exit__", [](const py::object& /*self */, const py::object& /*exc_type*/,
                            const py::object& /*exc_value*/, const py::object& /*traceback*/) {
            if (should_notify_counter > 0) {
                should_notify_counter--;
            }
        });

    mod.def(
        PYUNREALSDK_STUBGEN_FUNC("notify_changes", "AbstractContextManager[None]"),
        []() { return ContextManager{}; },
        PYUNREALSDK_STUBGEN_DOCSTRING(
            "Context manager to automatically notify the engine when you edit an object.\n"
            "\n"
            "This essentially just automatically calls obj._post_edit_change_property() after\n"
            "every setattr.\n"
            "\n"
            "Note that this only tracks top-level changes, it cannot track changes to inner\n"
            "struct fields, You will have to manually call obj._post_edit_chain_property()\n"
            "for them.\n"
            "\n"
            "Returns:\n"
            "    A new context manager.\n"));
}

}  // namespace pyunrealsdk::unreal

#endif
