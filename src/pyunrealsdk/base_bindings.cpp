#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/base_bindings.h"
#include "pyunrealsdk/unreal_bindings/wrapped_struct.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

namespace pyunrealsdk {

using namespace unrealsdk::unreal;

namespace {

/**
 * @brief Finds a class by name, which may be fully qualified.
 *
 * @param name The class name.
 * @param fully_qualified If the class name is fully qualified, or nullopt to autodetect.
 * @return The class, or nullptr if not found.
 */
UClass* find_class_potentially_qualified(const std::wstring& name,
                                         std::optional<bool> fully_qualified) {
    if (!fully_qualified.has_value()) {
        fully_qualified = name.find_first_of(L".:") != std::string::npos;
    }

    if (fully_qualified.value()) {
        return find_class(name);
    }
    return find_class(FName{name});
}

/**
 * @brief Finds a script struct by name, which may be fully qualified.
 * @note If two structs (presumably from different packages) share a name, non fully qualified
 *       lookup returns an undefined instance.
 *
 * @param name The script struct name.
 * @param fully_qualified If the script struct name is fully qualified, or nullopt to autodetect.
 * @return The script struct, or nullptr if not found.
 */
UScriptStruct* find_scriptstruct_potentially_qualified(const std::wstring& name,
                                                       std::optional<bool> fully_qualified) {
    static std::unordered_map<FName, UScriptStruct*> scriptstruct_cache{};
    static const auto scriptstruct_cls = find_class<UScriptStruct>();

    // If the cache is empty, do an initial parse through all objects
    if (scriptstruct_cache.empty()) {
        for (const auto& obj : unrealsdk::gobjects()) {
            if (!obj->is_instance(scriptstruct_cls)) {
                continue;
            }

            // We'll let this overwrite on first pass
            // Duplicate names are undefined
            scriptstruct_cache[obj->Name] = reinterpret_cast<UScriptStruct*>(obj);
        }
    }

    if (!fully_qualified.has_value()) {
        fully_qualified = name.find_first_of(L".:") != std::string::npos;
    }

    // If not fully qualified, do a quick fname lookup
    if (!fully_qualified.value()) {
        const FName fname{name};
        return scriptstruct_cache.contains(fname) ? scriptstruct_cache[fname] : nullptr;
    }

    // Otherwise call find object
    auto obj = unrealsdk::find_object(scriptstruct_cls, name);
    if (obj == nullptr) {
        return nullptr;
    }
    auto found_struct = validate_type<UScriptStruct>(obj);

    // Don't overwrite an existing entry
    if (!scriptstruct_cache.contains(found_struct->Name)) {
        scriptstruct_cache[found_struct->Name] = found_struct;
    }

    return found_struct;
}

}  // namespace

void register_base_bindings(py::module_& mod) {
    mod.def("find_class", &find_class_potentially_qualified,
            "Finds a class by name.\n"
            "\n"
            "Args:\n"
            "    name: The class name.\n"
            "    fully_qualified: If the class name is fully qualified, or None (the default)\n"
            "                     to autodetect.\n"
            "Returns:\n"
            "    The class, or None if not found.",
            "name"_a, "fully_qualified"_a = std::nullopt);

    mod.def(
        "make_struct",
        [](const std::wstring& name, std::optional<bool> fully_qualified,
           const py::kwargs& kwargs) {
            auto type = find_scriptstruct_potentially_qualified(name, fully_qualified);
            if (type == nullptr) {
                throw py::value_error(unrealsdk::fmt::format("Couldn't find script struct {}",
                                                             unrealsdk::utils::narrow(name)));
            }

            const py::args empty_args{};
            return unreal::make_struct(type, empty_args, kwargs);
        },
        "Finds and constructs a WrappedStruct by name.\n"
        "\n"
        "Args:\n"
        "    name: The struct name.\n"
        "    fully_qualified: If the struct name is fully qualified, or None (the\n"
        "                     default) to autodetect.\n"
        "    **kwargs: Fields on the struct to initialize.\n"
        "Returns:\n"
        "    The newly constructed struct.",
        "name"_a, "fully_qualified"_a = std::nullopt, py::pos_only{});

    mod.def(
        "find_object",
        [](const std::variant<UClass*, std::wstring>& cls_arg, const std::wstring& name) {
            auto cls_ptr = std::holds_alternative<UClass*>(cls_arg)
                               ? std::get<UClass*>(cls_arg)
                               : find_class_potentially_qualified(std::get<std::wstring>(cls_arg),
                                                                  std::nullopt);
            return unrealsdk::find_object(cls_ptr, name);
        },
        "Finds an object by name.\n"
        "\n"
        "Args:\n"
        "    cls: The object's class, or class name. If given as the name, always\n"
        "         autodetects if fully qualified - call find_class() directly if you need\n"
        "         to specify.\n"
        "    name: The object's name.\n"
        "Returns:\n"
        "    The object, or None if not found.",
        "cls"_a, "name"_a);

    mod.def(
        "find_all",
        [](const std::variant<UClass*, std::wstring>& cls_arg, bool exact) {
            auto cls_ptr = std::holds_alternative<UClass*>(cls_arg)
                               ? std::get<UClass*>(cls_arg)
                               : find_class_potentially_qualified(std::get<std::wstring>(cls_arg),
                                                                  std::nullopt);

            auto gobjects = unrealsdk::gobjects();

            std::vector<UObject*> results{};
            if (exact) {
                std::copy_if(gobjects.begin(), gobjects.end(), std::back_inserter(results),
                             [cls_ptr](UObject* obj) { return obj->Class == cls_ptr; });
            } else {
                std::copy_if(gobjects.begin(), gobjects.end(), std::back_inserter(results),
                             [cls_ptr](UObject* obj) { return obj->is_instance(cls_ptr); });
            }

            return results;
        },
        "Finds all instances of a class.\n"
        "\n"
        "Args:\n"
        "    cls: The object's class, or class name. If given as the name, always\n"
        "         autodetects if fully qualified - call find_class() directly if you need\n"
        "         to specify.\n"
        "    exact: If true (the default), only finds exact class matches. If false, also\n"
        "           matches subclasses.\n"
        "Returns:\n"
        "    A list of all instances of the class.",
        "cls"_a, "exact"_a = true);

    mod.def(
        "construct_object",
        [](const std::variant<UClass*, std::wstring>& cls_arg, UObject* outer, const FName& name,
           decltype(UObject::ObjectFlags) flags, UObject* template_obj) {
            auto cls_ptr = std::holds_alternative<UClass*>(cls_arg)
                               ? std::get<UClass*>(cls_arg)
                               : find_class_potentially_qualified(std::get<std::wstring>(cls_arg),
                                                                  std::nullopt);

            return unrealsdk::construct_object(cls_ptr, outer, name, flags, template_obj);
        },
        "Constructs a new object\n"
        "\n"
        "Args:\n"
        "    cls: The class to construct, or it's name. Required. If given as the name,\n"
        "         always autodetects if fully qualified - call find_class() directly if\n"
        "         you need to specify.\n"
        "    outer: The outer object to construct the new object under. Required.\n"
        "    name: The new object's name.\n"
        "    flags: Object flags to set.\n"
        "    template_obj: The template object to use.\n"
        "Returns:\n"
        "    The constructed object.\n",
        "cls"_a, "outer"_a, "name"_a = FName{0, 0}, "flags"_a = 0, "template_obj"_a = nullptr);
}

}  // namespace pyunrealsdk
