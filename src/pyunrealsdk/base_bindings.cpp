#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/base_bindings.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unrealsdk.h"

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
 * @brief Gets a class from a python object which may be the class itself, or it's name.
 * @note Throws if the class can't be found.
 *
 * @param cls The python object.
 * @return The class.
 */
UClass* find_class_potentially_given(const py::object& cls) {
    if (py::isinstance<UClass>(cls)) {
        return py::cast<UClass*>(cls);
    }

    auto class_name = py::cast<std::wstring>(cls);
    auto class_ptr = find_class_potentially_qualified(class_name, std::nullopt);
    if (class_ptr == nullptr) {
        throw py::value_error(unrealsdk::fmt::format("Couldn't find class {}", class_name));
    }
    return class_ptr;
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
        "find_object",
        [](const py::object& cls, const std::wstring& name) {
            return unrealsdk::find_object(find_class_potentially_given(cls), name);
        },
        "Finds an object by name.\n"
        "\n"
        "Args:\n"
        "    cls: The object's class, or class name. If given as the name, always\n"
        "         autodetects if fully qualified - call find_class directly if you need\n"
        "         to specify.\n"
        "    name: The object's name.\n"
        "Returns:\n"
        "    The object, or None if not found.",
        "cls"_a, "name"_a);

    mod.def(
        "find_all",
        [](const py::object& cls, bool exact) {
            auto class_ptr = find_class_potentially_given(cls);

            auto gobjects = unrealsdk::gobjects();

            std::vector<UObject*> results{};
            if (exact) {
                std::copy_if(gobjects.begin(), gobjects.end(), std::back_inserter(results),
                             [class_ptr](UObject* obj) { return obj->Class == class_ptr; });
            } else {
                std::copy_if(gobjects.begin(), gobjects.end(), std::back_inserter(results),
                             [class_ptr](UObject* obj) { return obj->is_instance(class_ptr); });
            }

            return results;
        },
        "Finds all instances of a class.\n"
        "\n"
        "Args:\n"
        "    cls: The object's class, or class name. If given as the name, always\n"
        "         autodetects if fully qualified - call find_class directly if you need\n"
        "         to specify.\n"
        "    exact: If true (the default), only finds exact class matches. If false, also\n"
        "           matches subclasses.\n"
        "Returns:\n"
        "    A list of all instances of the class.",
        "cls"_a, "exact"_a = true);

    mod.def("construct_object", &unrealsdk::construct_object,
            "Constructs a new object\n"
            "\n"
            "Args:\n"
            "    cls: The class to construct. Required.\n"
            "    outer: The outer object to construct the new object under. Required.\n"
            "    name: The new object's name.\n"
            "    flags: Object flags to set.\n"
            "    template_obj: The template object to use.\n"
            "Returns:\n"
            "    The constructed object.\n",
            "cls"_a, "outer"_a, "name"_a = FName{0, 0}, "flags"_a = 0, "template_obj"_a = nullptr);
}

}  // namespace pyunrealsdk
