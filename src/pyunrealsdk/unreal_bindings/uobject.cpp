#include "pyunrealsdk/unreal_bindings/uobject.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "unrealsdk/format.h"
#include "unrealsdk/unreal/cast_prop.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uobject_funcs.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/classes/ustruct_funcs.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unrealsdk.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

namespace {

/**
 * @brief Implementation of `UObject.__dir__`.
 *
 * @param self The object this is being called on.
 * @return The list of attributes this object supports.
 */
std::vector<std::string> uobject_dir(UObject* self) {
    // Start by calling the base dir function
    auto names = py::cast<std::vector<std::string>>(
        py::module_::import("builtins").attr("object").attr("__dir__")(self));

    // Append our fields
    auto fields = self->Class->fields();
    std::transform(fields.begin(), fields.end(), std::back_inserter(names),
                   [](auto obj) { return obj->Name; });

    return names;
}

/**
 * @brief Implementation of `UObject.__getattr__`.
 *
 * @param self The object this is being called on.
 * @param key The key of the attribute being gotten.
 * @return The value of the attribute.
 */
py::object uobject_getattr(UObject* self, const py::object& key) {
    // We can't push these at a higher scope because we need them to only run after the
    // sdk's been initialized
    static const UClass* uproperty_class = find_class(L"Property"_fn);
    static const UClass* ufunction_class = find_class(L"Function"_fn);

    UField* field = get_field_from_py_key(key, self->Class);

    if (field->is_instance(uproperty_class)) {
        // If we have a static array, return it as a tuple
        // Store in a list for now so we can still append.
        py::list ret{};

        cast_prop(reinterpret_cast<UProperty*>(field), [&self, &ret]<typename T>(const T* prop) {
            for (size_t i = 0; i < (size_t)prop->ArrayDim; i++) {
                ret.append(self->get<T>(prop, i));
            }
        });

        switch (ret.size()) {
            case 0:
                throw py::attribute_error(
                    unrealsdk::fmt::format("attribute '{}' has size of 0", field->Name));
            case 1:
                return ret[0];
            default:
                return py::tuple(ret);
        }
    } else if (field->is_instance(ufunction_class)) {
        auto func = self->get<UFunction, BoundFunction>(reinterpret_cast<UFunction*>(field));
        return py::cast(func);
    }

    throw py::attribute_error(unrealsdk::fmt::format("attribute '{}' has unknown type '{}'",
                                                     field->Name, field->Class->Name));
}

/**
 * @brief Implementation of `UObject.__setattr__`.
 *
 * @param self The object this is being called on.
 * @param key The key of the attribute being set.
 * @param value The new value of the attribute.
 */
void uobject_setattr(UObject* self, const py::object& key, const py::object& value) {
    // We can't put this at a higher scope because we need it to only run after the
    // sdk's been initialized
    static const UClass* uproperty_class = find_class(L"Property"_fn);

    UField* field = get_field_from_py_key(key, self->Class);
    if (!field->is_instance(uproperty_class)) {
        throw py::attribute_error(unrealsdk::fmt::format(
            "attribute '{}' is not a property, and thus cannot be set", field->Name));
    }

    auto prop = reinterpret_cast<UProperty*>(field);

    py::sequence value_seq;
    if (prop->ArrayDim > 1) {
        if (!py::isinstance<py::sequence>(value)) {
            std::string value_type_name = py::str(py::type::of(value).attr("__name__"));
            throw py::type_error(unrealsdk::fmt::format(
                "attribute value has unexpected type '{}', expected a sequence", value_type_name));
        }
        value_seq = value;

        if (value_seq.size() > static_cast<size_t>(prop->ArrayDim)) {
            throw py::type_error(unrealsdk::fmt::format(
                "attribute value is too long, {} supports a maximum of {} values", prop->Name,
                prop->ArrayDim));
        }
    } else {
        value_seq = py::make_tuple(value);
    }

    cast_prop(prop, [&self, &value_seq]<typename T>(const T* prop) {
        using value_type = typename PropTraits<T>::Value;

        size_t seq_size = value_seq.size();
        size_t prop_size = prop->ArrayDim;

        // If we're default constructable, set all the missing fields to the default value
        // If we're not, require specifying all values
        // Do this before writing the given values so that we can error out without making changes
        if constexpr (std::is_default_constructible_v<value_type>) {
            for (size_t i = seq_size; i < prop_size; i++) {
                self->set<T>(prop, i, {});
            }
        } else {
            if (seq_size != prop_size) {
                throw py::type_error(unrealsdk::fmt::format(
                    "attribute value is too short, {} must be given as exactly {} values (no known "
                    "default to use when less are given)",
                    prop->Name, prop->ArrayDim));
            }
        }

        for (size_t i = 0; i < seq_size; i++) {
            self->set<T>(prop, i, py::cast<value_type>(value_seq[i]));
        }
    });
}

}  // namespace

void register_uobject(py::module_& module) {
    PyUEClass<UObject>(
        module, "UObject", "The base class of all unreal objects.",
        // Need dynamic attr to create a `__dict__`, so that we can handle `__dir__` properly
        py::dynamic_attr())
        .def_no_constructor()
        .def_get_address()
        .def(
            "__str__",
            [](UObject* self) {
                return unrealsdk::fmt::format("{}'{}'", self->Class->Name,
                                              unrealsdk::uobject_path_name(self));
            },
            "Gets this object's name.\n"
            "\n"
            "Returns:\n"
            "    This object's name.")
        .def("__dir__", &uobject_dir,
             "Gets the attributes which exist on this object.\n"
             "\n"
             "Includes both python attributes and unreal fields.\n"
             "\n"
             "Returns:\n"
             "    A list of attributes which exist on this object.")
        .def("__getattr__", &uobject_getattr,
             "Reads an unreal field off of the object.\n"
             "\n"
             "Usually called with a string holding the field name (as is done in regular\n"
             "attribute access), which automatically looks up the field.\n"
             "\n"
             "In performance critical situations, you can also look up the field beforehand\n"
             "via obj.Class.find(\"name\"), then pass the it directly to this function. This does\n"
             "not get validated, passing a field which doesn't exist on the object is\n"
             "undefined behaviour.\n"
             "\n"
             "Note that getattr() only supports string keys, when passing a field you must\n"
             "call this function directly.\n"
             "\n"
             "Args:\n"
             "    key: The field's name, or the field object itself.\n"
             "Returns:\n"
             "    The field's value.",
             "key"_a)
        .def("__setattr__", &uobject_setattr,
             "Writes a value to an unreal property.\n"
             "\n"
             "Usually called with a string holding the field name (as is done in regular\n"
             "attribute access), which automatically looks up the field.\n"
             "\n"
             "In performance critical situations, you can also look up the field beforehand\n"
             "via obj.Class._find(\"name\"), then pass the it directly to this function. This\n"
             "does not get validated, passing a field which doesn't exist on the object is\n"
             "undefined behaviour.\n"
             "\n"
             "Note that setattr() only supports string keys, when passing a field you must\n"
             "call this function directly.\n"
             "\n"
             "Args:\n"
             "    key: The field's name, or the field object itself.\n"
             "    value: The value to write.\n"
             "Returns:\n"
             "    The field's value.",
             "key"_a, "value"_a)
        .def_readwrite("ObjectFlags", &UObject::ObjectFlags)
        .def_readwrite("InternalIndex", &UObject::InternalIndex)
        .def_readwrite("Class", &UObject::Class)
        .def_readwrite("Name", &UObject::Name)
        .def_readwrite("Outer", &UObject::Outer);
}

}  // namespace pyunrealsdk::unreal
