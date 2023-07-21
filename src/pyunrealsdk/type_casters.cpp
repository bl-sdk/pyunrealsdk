#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/type_casters.h"
#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/classes/properties/uboolproperty.h"
#include "unrealsdk/unreal/classes/properties/uclassproperty.h"
#include "unrealsdk/unreal/classes/properties/uinterfaceproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"
#include "unrealsdk/unreal/classes/ublueprintgeneratedclass.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/structs/fname.h"

namespace pyunrealsdk::type_casters {

namespace {

using known_uobject_children = std::tuple<unrealsdk::unreal::UObject,
                                          unrealsdk::unreal::UField,
                                          unrealsdk::unreal::UStruct,
                                          unrealsdk::unreal::UClass,
                                          unrealsdk::unreal::UBlueprintGeneratedClass,
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

/**
 * @brief Tail recursive template searching for a matching class, to implement `downcast_unreal`.
 *
 * @tparam i The index in the tuple to look up - auto incrementing.
 * @param src The source object to try downcast.
 * @param type The downcast type to use.
 * @return The source object.
 */
template <size_t i = 0>
const void* downcast_unreal_impl(const unrealsdk::unreal::UObject* src,
                                 const std::type_info*& type) {
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

        return downcast_unreal_impl<i + 1>(src, type);
    }
}

}  // namespace

const void* downcast_unreal(const unrealsdk::unreal::UObject* src, const std::type_info*& type) {
    return downcast_unreal_impl(src, type);
}

}  // namespace pyunrealsdk::type_casters
