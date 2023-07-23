#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/type_casters.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/classes/uobject.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::type_casters {

const void* downcast_unreal(const UObject* src, const std::type_info*& type) {
    if (src != nullptr) {
        cast<cast_options<true, true>>(
            src,
            // On successful cast: return the templated type
            [&type]<typename T>(const T* /*obj*/) { type = &typeid(T); },
            // On error: fall back to UObject
            [&type](const UObject* /*obj*/) { type = &typeid(UObject); });
    }

    return src;
}

}  // namespace pyunrealsdk::type_casters
