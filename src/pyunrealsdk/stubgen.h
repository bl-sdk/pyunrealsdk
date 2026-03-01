#ifndef PYUNREALSDK_STUBGEN_H
#define PYUNREALSDK_STUBGEN_H

/*
These macros are used to generate the python type stubs - they're easier for us to match than fully
parsing C++. See also the stubgen readme.

All macros have a `_N` null macro variant, which expands to nothing.
*/

/**
 * @brief Sets what python module the following definitions are in.
 *
 * @note PYUNREALSDK_STUBGEN_MODULE("a.b.c") and PYUNREALSDK_STUBGEN_SUBMODULE("a.b", "c") set
 *       equivalent modules, but expand to different strings.
 *
 * @param outer (If available) the full outer module path, as a string.
 * @param name The module's name, as a string.
 * @return Expands to the name string.
 */
#define PYUNREALSDK_STUBGEN_MODULE(name) name
#define PYUNREALSDK_STUBGEN_MODULE_N(name)
#define PYUNREALSDK_STUBGEN_SUBMODULE(outer, name) name
#define PYUNREALSDK_STUBGEN_SUBMODULE_N(outer, name)

/**
 * @brief Defines an attribute, attaching to the last class, enum, or module.
 *
 * @param name The attribute's name, as a string.
 * @param type The type hint to give this attribute, as a string.
 * @return Expands to the name string.
 */
#define PYUNREALSDK_STUBGEN_ATTR(name, type) name
#define PYUNREALSDK_STUBGEN_ATTR_N(name, type)
#define PYUNREALSDK_STUBGEN_READONLY_PROP(name, type) name
#define PYUNREALSDK_STUBGEN_READONLY_PROP_N(name, type)

/**
 * @brief Defines a function or method, attaching to the last module or class
 *
 * @param name The function's name, as a string.
 * @param ret The function's python return type, as a string.
 * @return Expands to the name string.
 */
#define PYUNREALSDK_STUBGEN_FUNC(name, ret_type) name
#define PYUNREALSDK_STUBGEN_FUNC_N(name, ret_type)
#define PYUNREALSDK_STUBGEN_METHOD(name, ret_type) name
#define PYUNREALSDK_STUBGEN_METHOD_N(name, ret_type)
#define PYUNREALSDK_STUBGEN_STATICMETHOD(name, ret_type) name
#define PYUNREALSDK_STUBGEN_STATICMETHOD_N(name, ret_type)
#define PYUNREALSDK_STUBGEN_CLASSMETHOD(name, ret_type) name
#define PYUNREALSDK_STUBGEN_CLASSMETHOD_N(name, ret_type)

/**
 * @brief Defines a function overload, attaching to the last function or method.
 * @note Acts as a new function of the same type as the one it attached to.
 *
 * @param name The overload's name, as a string. Must be identical to the previous function.
 * @param ret The overload's python return type, as a string.
 * @return Expands to the name string.
 */
#define PYUNREALSDK_STUBGEN_OVERLOAD(name, ret_type) name
#define PYUNREALSDK_STUBGEN_OVERLOAD_N(name, ret_type)

/**
 * @brief Defines a docstring, attaching to the last relevant element.
 *
 * @param name The method's name, as a string.
 * @return Expands to the name string.
 */
#define PYUNREALSDK_STUBGEN_DOCSTRING(doc) doc
#define PYUNREALSDK_STUBGEN_DOCSTRING_N(doc)

/**
 * @brief Defines an arg, attaching to the last function or method.
 *
 * @param name The arg's name, as a pybind _a literal.
 * @param type The arg's type hint, as a string.
 * @param py_default If this arg has a default value, a string holding it's python equivalent.
 * @return Expands to the name string.
 */
#define PYUNREALSDK_STUBGEN_ARG(name, type, py_default) name
#define PYUNREALSDK_STUBGEN_ARG_N(name, type, py_default)

/**
 * @brief Marker for where pos-only/kw-only args start, attaching to the last function or method.
 *
 * @return Expands to an instance of the relevant pybind type.
 */
#define PYUNREALSDK_STUBGEN_POS_ONLY() (py::pos_only{})
#define PYUNREALSDK_STUBGEN_POS_ONLY_N()
#define PYUNREALSDK_STUBGEN_KW_ONLY() (py::kw_only{})
#define PYUNREALSDK_STUBGEN_KW_ONLY_N()

/**
 * @brief Defines an enum.
 *
 * @param name The enum's name, as a string.
 * @return Expands to the name string.
 */
#define PYUNREALSDK_STUBGEN_ENUM(name) name
#define PYUNREALSDK_STUBGEN_ENUM_N(name)

/**
 * @brief Defines a class.
 *
 * @param name The class's name, as a string.
 * @param super The class's superclass(es) as a string, if it has any.
 * @return Expands to the name string.
 */
#define PYUNREALSDK_STUBGEN_CLASS(name, super) name
#define PYUNREALSDK_STUBGEN_CLASS_N(name, super)

/**
 * @brief Adds the deprecated decorator to the previous class, function, or method.
 *
 * @param msg The deprecation message.
 * @return Expands to the deprecation message.
 */
#define PYUNREALSDK_STUBGEN_DEPRECATED(msg) msg
#define PYUNREALSDK_STUBGEN_DEPRECATED_N(msg)

/**
 * @brief Marks the previous class, function, or method as generic.
 *
 * @param types The generic types, including the square brackets.
 */
#define PYUNREALSDK_STUBGEN_GENERIC(types)
#define PYUNREALSDK_STUBGEN_GENERIC_N(types)

/**
 * @brief Defines a method with the signature 'def name(self, *args: Any, **kwargs: Any) -> Never:'
 *
 * @param name The function's name, as a string.
 * @return Expands to the name string.
 */
#define PYUNREALSDK_STUBGEN_NEVER_METHOD(name)    \
    PYUNREALSDK_STUBGEN_METHOD(name, "Never")     \
    PYUNREALSDK_STUBGEN_ARG_N("*args"_a, "Any", ) \
    PYUNREALSDK_STUBGEN_ARG_N("**kwargs"_a, "Any", )
#define PYUNREALSDK_STUBGEN_NEVER_METHOD_N(name)  \
    PYUNREALSDK_STUBGEN_METHOD_N(name, "Never")   \
    PYUNREALSDK_STUBGEN_ARG_N("*args"_a, "Any", ) \
    PYUNREALSDK_STUBGEN_ARG_N("**kwargs"_a, "Any", )

#endif /* PYUNREALSDK_STUBGEN_H */
