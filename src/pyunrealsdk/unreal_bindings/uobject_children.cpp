#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/uobject_children.h"
#include "pyunrealsdk/stubgen.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "unrealsdk/unreal/classes/ublueprintgeneratedclass.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uconst.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/properties/attribute_property.h"
#include "unrealsdk/unreal/properties/copyable_property.h"
#include "unrealsdk/unreal/properties/persistent_object_ptr_property.h"
#include "unrealsdk/unreal/properties/zarrayproperty.h"
#include "unrealsdk/unreal/properties/zboolproperty.h"
#include "unrealsdk/unreal/properties/zbyteproperty.h"
#include "unrealsdk/unreal/properties/zclassproperty.h"
#include "unrealsdk/unreal/properties/zcomponentproperty.h"
#include "unrealsdk/unreal/properties/zdelegateproperty.h"
#include "unrealsdk/unreal/properties/zenumproperty.h"
#include "unrealsdk/unreal/properties/zgbxdefptrproperty.h"
#include "unrealsdk/unreal/properties/zinterfaceproperty.h"
#include "unrealsdk/unreal/properties/zmulticastdelegateproperty.h"
#include "unrealsdk/unreal/properties/zobjectproperty.h"
#include "unrealsdk/unreal/properties/zproperty.h"
#include "unrealsdk/unreal/properties/zstrproperty.h"
#include "unrealsdk/unreal/properties/zstructproperty.h"
#include "unrealsdk/unreal/properties/ztextproperty.h"
#include "unrealsdk/unreal/properties/zweakobjectproperty.h"
#include "unrealsdk/unreal/structs/ffield.h"
#include "unrealsdk/unreal/structs/tfieldvariant.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_uobject_children(py::module_& mod) {
    PYUNREALSDK_STUBGEN_MODULE_N("unrealsdk.unreal")

    // ======== Not technically subclasses but still closely related ========

    PyUEClass<FFieldClass>(mod, PYUNREALSDK_STUBGEN_CLASS("FFieldClass", ))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Name", "str"), &FFieldClass::Name)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("SuperField", "FFieldClass | None"),
                         &FFieldClass::SuperField)
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD("__new__"),
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of unreal objects.");
             })
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD("__init__"),
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of unreal objects.");
             })
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__repr__", "str"),
            [](FFieldClass* self) {
                // Make this kind of look like a normal class?
                return std::format("FFieldClass'{}'", self->Name());
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets this object's full name.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    This object's name.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_get_address", "int"),
            [](FFieldClass* self) { return reinterpret_cast<uintptr_t>(self); },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets the address of this object, for debugging.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    This object's address.\n"));

    PyUEClass<FField>(mod, PYUNREALSDK_STUBGEN_CLASS("FField", ))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Class", "UClass"), &FField::Class)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Next", "FField"), &FField::Next)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Name", "str"), &FField::Name)
        .def_property(
            PYUNREALSDK_STUBGEN_ATTR("Owner", "FField | UObject | None"),
            [](FField& self) {
                py::object ret;
                self.Owner().cast([&]<typename T>(T* obj) { ret = py::cast(obj); });
                return ret;
            },
            [](FField& self, std::variant<std::nullptr_t, UObject*, FField*> val) {
                if (std::holds_alternative<std::nullptr_t>(val)) {
                    self.Owner() = nullptr;
                } else if (std::holds_alternative<UObject*>(val)) {
                    self.Owner() = std::get<UObject*>(val);
                } else {
                    self.Owner() = std::get<FField*>(val);
                }
            })
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD("__new__"),
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of unreal objects.");
             })
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD("__init__"),
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of unreal objects.");
             })
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__repr__", "str"),
            [](FField* self) {
                return std::format("{}'{}'", self->Class()->Name(),
                                   unrealsdk::utils::narrow(self->get_path_name()));
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets this object's full name.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    This object's name.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_get_address", "int"),
            [](FField* self) { return reinterpret_cast<uintptr_t>(self); },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets the address of this object, for debugging.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    This object's address.\n"))
        .def(PYUNREALSDK_STUBGEN_METHOD("_path_name", "str"), &FField::get_path_name,
             PYUNREALSDK_STUBGEN_DOCSTRING("Gets this object's path name, excluding the class.\n"
                                           "\n"
                                           "Returns:\n"
                                           "    This object's name.\n"));

    // ======== First Layer Subclasses ========

    PyUEClass<UField, UObject>(mod, PYUNREALSDK_STUBGEN_CLASS("UField", "UObject"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Next", "UField | None"), &UField::Next);

    // ======== Second Layer Subclasses ========

    PyUEClass<UConst, UField>(mod, PYUNREALSDK_STUBGEN_CLASS("UConst", "UField"))
        // Deliberately not using def_member_prop, since we need to do extra string conversions
        .def_property(
            PYUNREALSDK_STUBGEN_ATTR("Value", "str"),
            [](const UConst* self) { return (std::string)self->Value(); },
            [](UConst* self, const std::string& new_value) { self->Value() = new_value; });

#if UNREALSDK_PROPERTIES_ARE_FFIELD
    PyUEClass<ZProperty, FField>(mod, PYUNREALSDK_STUBGEN_CLASS("ZProperty", "FField"))
#else
    PyUEClass<ZProperty, UField>(mod, PYUNREALSDK_STUBGEN_CLASS("ZProperty", "UField"))
#endif
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ArrayDim", "int"), &ZProperty::ArrayDim)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ElementSize", "int"), &ZProperty::ElementSize)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("PropertyFlags", "int"),
                         &ZProperty::PropertyFlags)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Offset_Internal", "int"),
                         &ZProperty::Offset_Internal)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("PropertyLinkNext", "ZProperty | None"),
                         &ZProperty::PropertyLinkNext);

    PyUEClass<UStruct, UField>(mod, PYUNREALSDK_STUBGEN_CLASS("UStruct", "UField"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("SuperField", "UStruct | None"),
                         &UStruct::SuperField)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Children", "UField | None"), &UStruct::Children)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("PropertySize", "int"), &UStruct::PropertySize)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("PropertyLink", "ZProperty | None"),
                         &UStruct::PropertyLink)
#if UNREALSDK_USTRUCT_HAS_ALIGNMENT
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("MinAlignment", "int"), &UStruct::MinAlignment)
#endif
#if UNREALSDK_PROPERTIES_ARE_FFIELD
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ChildProperties", "FField | None"),
                         &UStruct::MinAlignment)
#endif
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_fields", "Iterator[UField]"),
            [](UStruct* self) {
                auto fields = self->fields();
                // I don't like how calling this with the `fields` directly takes an lvalue
                // reference - would prefer to move an rvalue.
                // Testing, everything still works fine, there's no memory leak, but prefer to
                // manually call this with the iterators anyway
                return py::make_iterator(fields.begin(), fields.end());
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Iterates over all fields in the struct.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    An iterator over all fields in the struct.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_properties", "Iterator[ZProperty]"),
            [](UStruct* self) {
                auto props = self->properties();
                return py::make_iterator(props.begin(), props.end());
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Iterates over all properties in the struct.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    An iterator over all properties in the struct.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_superfields", "Iterator[UStruct]"),
            [](UStruct* self) {
                auto superfields = self->superfields();
                return py::make_iterator(superfields.begin(), superfields.end());
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Iterates over this struct and it's superfields.\n"
                                          "\n"
                                          "Note this includes this struct itself.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    An iterator over all superfields in the struct.\n"))
        .def(PYUNREALSDK_STUBGEN_METHOD("_inherits", "bool"), &UStruct::inherits,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Checks if this structs inherits from another.\n"
                 "\n"
                 "Also returns true if this struct *is* the given struct.\n"
                 "\n"
                 "Args:\n"
                 "    base_struct: The base struct to check if this inherits from.\n"
                 "Returns:\n"
                 "    True if this struct is the given struct, or inherits from it.\n"),
             PYUNREALSDK_STUBGEN_ARG("base_struct"_a, "UStruct", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("_get_struct_size", "int"), &UStruct::get_struct_size,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Gets the actual size of the described structure, including alignment.\n"
                 "\n"
                 "Returns:\n"
                 "    The size which must be allocated.\n"))
        .def(
#ifdef UNREALSDK_PROPERTIES_ARE_FFIELD
            PYUNREALSDK_STUBGEN_METHOD("_find", "UField | ZProperty"),
#else
            PYUNREALSDK_STUBGEN_METHOD("_find", "UField"),
#endif
            [](UStruct* self, FName name) {
                auto ret = self->find(name);

                // UFields are UObjects
                auto field = ret.as_uobject();
                if (field != nullptr) {
                    return py::cast(field);
                }

                // Properties might be FFields (if enabled)
                auto prop = ret.as_ffield();
                if (prop != nullptr) {
                    return py::cast(prop);
                }

                // This should be impossible, because the find class should throw first
                throw std::invalid_argument("Couldn't find field " + (std::string)name);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Finds a child field by name.\n"
                                          "\n"
                                          "Throws an exception if the child is not found.\n"
                                          "\n"
                                          "Args:\n"
                                          "    name: The name of the child field.\n"
                                          "Returns:\n"
                                          "    The found child field.\n"),
            PYUNREALSDK_STUBGEN_ARG("name"_a, "str", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("_find_prop", "ZProperty"), &UStruct::find_prop,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Finds a child property by name.\n"
                 "\n"
                 "When known to be a property, this is more efficient than _find.\n"
                 "\n"
                 "Throws an exception if the child is not found.\n"
                 "\n"
                 "Args:\n"
                 "    name: The name of the child property.\n"
                 "Returns:\n"
                 "    The found child property.\n"),
             PYUNREALSDK_STUBGEN_ARG("name"_a, "str", ));

    // ======== Third Layer Subclasses ========

    PyUEClass<ZGbxDefPtrProperty, ZProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZGbxDefPtrProperty", "ZProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Struct", "UScriptStruct"),
                         &ZGbxDefPtrProperty::Struct);

    PyUEClass<ZArrayProperty, ZProperty>(mod,
                                         PYUNREALSDK_STUBGEN_CLASS("ZArrayProperty", "ZProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Inner", "ZProperty"), &ZArrayProperty::Inner);

    PyUEClass<ZBoolProperty, ZProperty>(mod,
                                        PYUNREALSDK_STUBGEN_CLASS("ZBoolProperty", "ZProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("FieldMask", "int"), &ZBoolProperty::FieldMask);

    PyUEClass<ZByteProperty, ZProperty>(mod,
                                        PYUNREALSDK_STUBGEN_CLASS("ZByteProperty", "ZProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Enum", "UEnum | None"), &ZByteProperty::Enum);

    PyUEClass<UClass, UStruct>(mod, PYUNREALSDK_STUBGEN_CLASS("UClass", "UStruct"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ClassDefaultObject", "UObject"),
                         &UClass::ClassDefaultObject)
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_implements", "bool"),
            [](UClass* self, UClass* interface) { return self->implements(interface, nullptr); },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Checks if this class implements a given interface.\n"
                "\n"
                "Args:\n"
                "    interface: The interface to check.\n"
                "Returns:\n"
                "    True if this class implements the interface, false otherwise.\n"),
            PYUNREALSDK_STUBGEN_ARG("interface"_a, "UClass", ))
        .def_property_readonly(PYUNREALSDK_STUBGEN_READONLY_PROP("Interfaces", "list[UClass]"),
                               [](const UClass* self) {
                                   std::vector<UClass*> interfaces{};
                                   for (const auto& iface : self->Interfaces()) {
                                       interfaces.push_back(iface.Class);
                                   }
                                   return interfaces;
                               });

    PyUEClass<ZDelegateProperty, ZProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZDelegateProperty", "ZProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Signature", "UFunction"),
                         &ZDelegateProperty::Signature);

    PyUEClass<ZDoubleProperty, ZProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZDoubleProperty", "ZProperty"));

    PyUEClass<ZEnumProperty, ZProperty>(mod,
                                        PYUNREALSDK_STUBGEN_CLASS("ZEnumProperty", "ZProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("UnderlyingProp", "ZProperty"),
                         &ZEnumProperty::UnderlyingProp)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Enum", "UEnum"), &ZEnumProperty::Enum);

    PyUEClass<ZFloatProperty, ZProperty>(mod,
                                         PYUNREALSDK_STUBGEN_CLASS("ZFloatProperty", "ZProperty"));

    PyUEClass<UFunction, UStruct>(mod, PYUNREALSDK_STUBGEN_CLASS("UFunction", "UStruct"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("FunctionFlags", "int"),
                         &UFunction::FunctionFlags)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("NumParams", "int"), &UFunction::NumParams)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ParamsSize", "int"), &UFunction::ParamsSize)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ReturnValueOffset", "int"),
                         &UFunction::ReturnValueOffset)
        .def(PYUNREALSDK_STUBGEN_METHOD("_find_return_param", "ZProperty"),
             &UFunction::find_return_param,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Finds the return param for this function (if it exists).\n"
                 "\n"
                 "Returns:\n"
                 "    The return param, or None if it doesn't exist.\n"));

    PyUEClass<ZInt8Property, ZProperty>(mod,
                                        PYUNREALSDK_STUBGEN_CLASS("ZInt8Property", "ZProperty"));

    PyUEClass<ZInt16Property, ZProperty>(mod,
                                         PYUNREALSDK_STUBGEN_CLASS("ZInt16Property", "ZProperty"));

    PyUEClass<ZInt64Property, ZProperty>(mod,
                                         PYUNREALSDK_STUBGEN_CLASS("ZInt64Property", "ZProperty"));

    PyUEClass<ZInterfaceProperty, ZProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZInterfaceProperty", "ZProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("InterfaceClass", "UClass"),
                         &ZInterfaceProperty::InterfaceClass);

    PyUEClass<ZIntProperty, ZProperty>(mod, PYUNREALSDK_STUBGEN_CLASS("ZIntProperty", "ZProperty"));

    PyUEClass<ZMulticastDelegateProperty, ZProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZMulticastDelegateProperty", "ZProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Signature", "UFunction"),
                         &ZMulticastDelegateProperty::Signature);

    PyUEClass<ZNameProperty, ZProperty>(mod,
                                        PYUNREALSDK_STUBGEN_CLASS("ZNameProperty", "ZProperty"));

    PyUEClass<ZObjectProperty, ZProperty>(mod,
                                          PYUNREALSDK_STUBGEN_CLASS("ZObjectProperty", "ZProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("PropertyClass", "UClass"),
                         &ZObjectProperty::PropertyClass);

    PyUEClass<UScriptStruct, UStruct>(mod, PYUNREALSDK_STUBGEN_CLASS("UScriptStruct", "UStruct"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("StructFlags", "int"),
                         &UScriptStruct::StructFlags);

    PyUEClass<ZStrProperty, ZProperty>(mod, PYUNREALSDK_STUBGEN_CLASS("ZStrProperty", "ZProperty"));

    PyUEClass<ZStructProperty, ZProperty>(mod,
                                          PYUNREALSDK_STUBGEN_CLASS("ZStructProperty", "ZProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Struct", "UScriptStruct"),
                         &ZStructProperty::Struct);

    PyUEClass<ZTextProperty, ZProperty>(mod,
                                        PYUNREALSDK_STUBGEN_CLASS("ZTextProperty", "ZProperty"));

    PyUEClass<ZUInt16Property, ZProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZUInt16Property", "ZProperty"));

    PyUEClass<ZUInt32Property, ZProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZUInt32Property", "ZProperty"));

    PyUEClass<ZUInt64Property, ZProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZUInt64Property", "ZProperty"));

    // ======== Fourth Layer Subclasses ========

    PyUEClass<UBlueprintGeneratedClass, UClass>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UBlueprintGeneratedClass", "UClass"));

    PyUEClass<ZByteAttributeProperty, ZByteProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZByteAttributeProperty", "ZByteProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ModifierStackProperty", "ZArrayProperty"),
                         &ZByteAttributeProperty::ModifierStackProperty)
        .def_member_prop(
            PYUNREALSDK_STUBGEN_ATTR("OtherAttributeProperty", "ZByteAttributeProperty"),
            &ZByteAttributeProperty::OtherAttributeProperty);

    PyUEClass<ZClassProperty, ZObjectProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZClassProperty", "ZObjectProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("MetaClass", "UClass"),
                         &ZClassProperty::MetaClass);

    PyUEClass<ZComponentProperty, ZObjectProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZComponentProperty", "ZObjectProperty"));

    PyUEClass<ZFloatAttributeProperty, ZFloatProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZFloatAttributeProperty", "ZFloatProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ModifierStackProperty", "ZArrayProperty"),
                         &ZFloatAttributeProperty::ModifierStackProperty)
        .def_member_prop(
            PYUNREALSDK_STUBGEN_ATTR("OtherAttributeProperty", "ZFloatAttributeProperty"),
            &ZFloatAttributeProperty::OtherAttributeProperty);

    PyUEClass<ZIntAttributeProperty, ZIntProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZIntAttributeProperty", "ZIntProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ModifierStackProperty", "ZArrayProperty"),
                         &ZIntAttributeProperty::ModifierStackProperty)
        .def_member_prop(
            PYUNREALSDK_STUBGEN_ATTR("OtherAttributeProperty", "ZIntAttributeProperty"),
            &ZIntAttributeProperty::OtherAttributeProperty);

    // ZLazyObjectProperty - registered elsewhere
    // ZSoftObjectProperty - registered elsewhere

    PyUEClass<ZWeakObjectProperty, ZObjectProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("ZWeakObjectProperty", "ZObjectProperty"));

    // ======== Fifth Layer Subclasses ========

    // ZSoftClassProperty - registered elsewhere

    // ======== Deprecated UProperty Aliases ========

    // In pyunrealsdk 1.9.0, we changed all `UProperty` types to `ZProperty` types. Add back aliases
    // for the old names.
    // In Python, we just do this using an attribute level copy
    // In the stubs, pretend it's a subclass so that we can attach a deprecated marker

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_DEPRECATED_PROPERTY_ALIAS(name_without_prefix, parent_class)      \
    PYUNREALSDK_STUBGEN_CLASS_N("U" name_without_prefix,                          \
                                "Z" name_without_prefix ", " parent_class)        \
    PYUNREALSDK_STUBGEN_DEPRECATED_N("U" name_without_prefix                      \
                                     " has been renamed to Z" name_without_prefix \
                                     ", this is a deprecated alias")              \
    mod.attr("U" name_without_prefix) = mod.attr("Z" name_without_prefix);

#if UNREALSDK_PROPERTIES_ARE_FFIELD
    DECLARE_DEPRECATED_PROPERTY_ALIAS("Property", "FField")
#else
    DECLARE_DEPRECATED_PROPERTY_ALIAS("Property", "UField")
#endif

    // This only needs the types which were available in pyunrealsdk 1.8.0 - no need to add new ones
    DECLARE_DEPRECATED_PROPERTY_ALIAS("ArrayProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("BoolProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("ByteAttributeProperty", "UByteProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("ByteProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("ClassProperty", "UObjectProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("ComponentProperty", "UObjectProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("DelegateProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("DoubleProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("EnumProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("FloatAttributeProperty", "UByteProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("FloatProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("Int16Property", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("Int64Property", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("Int8Property", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("IntAttributeProperty", "UByteProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("IntProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("InterfaceProperty", "UProperty")
    // LazyObjectProperty - registered elsewhere
    DECLARE_DEPRECATED_PROPERTY_ALIAS("MulticastDelegateProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("NameProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("ObjectProperty", "UProperty")
    // SoftClassProperty - registered elsewhere
    // SoftObjectProperty - registered elsewhere
    DECLARE_DEPRECATED_PROPERTY_ALIAS("StrProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("StructProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("TextProperty", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("UInt16Property", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("UInt32Property", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("UInt64Property", "UProperty")
    DECLARE_DEPRECATED_PROPERTY_ALIAS("WeakObjectProperty", "UObjectProperty")
}

}  // namespace pyunrealsdk::unreal

#endif
