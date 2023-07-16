#ifndef PYUNREALSDK_TYPE_CASTERS_H
#define PYUNREALSDK_TYPE_CASTERS_H

// This file is included in the pch, so we can't include it here, just need to assume we have access
// to everything we need already

#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/classes/properties/uboolproperty.h"
#include "unrealsdk/unreal/classes/properties/uclassproperty.h"
#include "unrealsdk/unreal/classes/properties/uinterfaceproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/structs/fname.h"

namespace pybind11 {

// Make the UObject hierarchy automatically downcast
namespace {

using known_uobject_children = std::tuple<unrealsdk::unreal::UObject,
                                          unrealsdk::unreal::UField,
                                          unrealsdk::unreal::UStruct,
                                          unrealsdk::unreal::UClass,
                                          unrealsdk::unreal::UFunction,
                                          unrealsdk::unreal::UScriptStruct,
                                          unrealsdk::unreal::UProperty,
                                          unrealsdk::unreal::UInt8Property,
                                          unrealsdk::unreal::UInt16Property,
                                          unrealsdk::unreal::UIntProperty,
                                          unrealsdk::unreal::UInt64Property,
                                          unrealsdk::unreal::UByteProperty,
                                          unrealsdk::unreal::UUInt16Property,
                                          unrealsdk::unreal::UUInt32Property,
                                          unrealsdk::unreal::UUInt64Property,
                                          unrealsdk::unreal::UFloatProperty,
                                          unrealsdk::unreal::UDoubleProperty,
                                          unrealsdk::unreal::UNameProperty,
                                          unrealsdk::unreal::UArrayProperty,
                                          unrealsdk::unreal::UBoolProperty,
                                          unrealsdk::unreal::UInterfaceProperty,
                                          unrealsdk::unreal::UObjectProperty,
                                          unrealsdk::unreal::UStrProperty,
                                          unrealsdk::unreal::UStructProperty,
                                          unrealsdk::unreal::UClassProperty>;

}  // namespace

template <typename itype>
struct polymorphic_type_hook<
    itype,
    detail::enable_if_t<std::is_base_of<unrealsdk::unreal::UObject, itype>::value>> {
    template <int i = 0>
    static const void* get(const unrealsdk::unreal::UObject* src, const std::type_info*& type) {
        if (src == nullptr) {
            type = nullptr;
            return src;
        }

        if constexpr (i >= std::tuple_size_v<known_uobject_children>) {
            // Fallback to object
            type = &typeid(unrealsdk::unreal::UObject);
            return src;
        } else {
            using cls = std::tuple_element_t<i, known_uobject_children>;

            if (src->Class->Name == unrealsdk::unreal::cls_fname<cls>()) {
                type = &typeid(cls);
                return src;
            }

            return get<i + 1>(src, type);
        }
    }
};

namespace detail {

// Allow FNames to cast to/from python strings
template <>
struct type_caster<unrealsdk::unreal::FName> {
   public:
    // The name we set here is used in the signatures added to the docstring, so set it as `str` so
    // they appear valid
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory, readability-identifier-length)
    PYBIND11_TYPE_CASTER(unrealsdk::unreal::FName, py::detail::const_name("str"));

    bool load(handle src, bool /*convert*/) {
        py::ssize_t size = 0;
        const char* str = PyUnicode_AsUTF8AndSize(src.ptr(), &size);

        if (str == nullptr) {
            return false;
        }

        value = unrealsdk::unreal::FName(std::string{str, static_cast<size_t>(size)});
        return true;
    }

    static handle cast(unrealsdk::unreal::FName src,
                       return_value_policy /* policy */,
                       handle /* parent */) {
        std::string name = src;
        return PyUnicode_FromStringAndSize(name.c_str(), static_cast<py::ssize_t>(name.size()));
    }
};

}  // namespace detail

}  // namespace pybind11

#endif /* PYUNREALSDK_TYPE_CASTERS_H */
