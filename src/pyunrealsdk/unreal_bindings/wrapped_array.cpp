#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/wrapped_array.h"
#include "pyunrealsdk/stubgen.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_wrapped_array(py::module_& mod) {
    PYUNREALSDK_STUBGEN_MODULE_N("unrealsdk.unreal")

    auto cls = py::classh<WrappedArray>(mod, PYUNREALSDK_STUBGEN_CLASS("WrappedArray", ));
    PYUNREALSDK_STUBGEN_GENERIC_N("[T = Any]")

    cls.def(PYUNREALSDK_STUBGEN_NEVER_METHOD("__new__"), &impl::array_py_new)
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD_N("__init__") py::init(&impl::array_py_init))
        .def(PYUNREALSDK_STUBGEN_METHOD("__repr__", "str"), &impl::array_py_repr,
             PYUNREALSDK_STUBGEN_DOCSTRING("Gets a string representation of this array.\n"
                                           "\n"
                                           "Returns:\n"
                                           "    The string representation.\n"))
            PYUNREALSDK_STUBGEN_NEVER_METHOD_N("__getitem__")
        .def(PYUNREALSDK_STUBGEN_OVERLOAD("__getitem__", "T"), &impl::array_py_getitem,
             PYUNREALSDK_STUBGEN_DOCSTRING("Gets an item from the array.\n"
                                           "\n"
                                           "Args:\n"
                                           "    idx: The index to get.\n"
                                           "Returns:\n"
                                           "    The item at the given index.\n"),
             PYUNREALSDK_STUBGEN_ARG("idx"_a, "int", ))
        .def(PYUNREALSDK_STUBGEN_OVERLOAD("__getitem__", "list[T]"), &impl::array_py_getitem_slice,
             PYUNREALSDK_STUBGEN_DOCSTRING("Gets a range from the array.\n"
                                           "\n"
                                           "Args:\n"
                                           "    range: The range to get.\n"
                                           "Returns:\n"
                                           "    The items in the given range.\n"),
             PYUNREALSDK_STUBGEN_ARG("range"_a, "slice", ))
            PYUNREALSDK_STUBGEN_NEVER_METHOD_N("__setitem__")
        .def(PYUNREALSDK_STUBGEN_OVERLOAD("__setitem__", "None"), &impl::array_py_setitem,
             PYUNREALSDK_STUBGEN_DOCSTRING("Sets an item in the array.\n"
                                           "\n"
                                           "Args:\n"
                                           "    idx: The index to set.\n"
                                           "    value: The value to set.\n"),
             PYUNREALSDK_STUBGEN_ARG("idx"_a, "int", ), PYUNREALSDK_STUBGEN_ARG("value"_a, "T", ))
        .def(PYUNREALSDK_STUBGEN_OVERLOAD("__setitem__", "None"), &impl::array_py_setitem_slice,
             PYUNREALSDK_STUBGEN_DOCSTRING("Sets a range of items in the array.\n"
                                           "\n"
                                           "Args:\n"
                                           "    range: The range to set.\n"
                                           "    value: The values to set.\n"),
             PYUNREALSDK_STUBGEN_ARG("range"_a, "slice", ),
             PYUNREALSDK_STUBGEN_ARG("value"_a, "Sequence[T]", ))
            PYUNREALSDK_STUBGEN_NEVER_METHOD_N("__delitem__")
        .def(PYUNREALSDK_STUBGEN_OVERLOAD("__delitem__", "None"), &impl::array_py_delitem,
             PYUNREALSDK_STUBGEN_DOCSTRING("Deletes an item from the array.\n"
                                           "\n"
                                           "Args:\n"
                                           "    idx: The index to delete.\n"),
             PYUNREALSDK_STUBGEN_ARG("idx"_a, "int", ))
        .def(PYUNREALSDK_STUBGEN_OVERLOAD("__delitem__", "None"), &impl::array_py_delitem_slice,
             PYUNREALSDK_STUBGEN_DOCSTRING("Deletes a range from the array.\n"
                                           "\n"
                                           "Args:\n"
                                           "    range: The range to delete.\n"),
             PYUNREALSDK_STUBGEN_ARG("range"_a, "slice", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("__len__", "int"), &WrappedArray::size,
             PYUNREALSDK_STUBGEN_DOCSTRING("Gets the length of the array.\n"
                                           "\n"
                                           "Returns:\n"
                                           "    The length of the array.\n"))
        .def(PYUNREALSDK_STUBGEN_METHOD("__iter__", "Iterator[T]"), &impl::array_py_iter,
             // Keep the array alive as long as the iterator is
             py::keep_alive<0, 1>(),
             PYUNREALSDK_STUBGEN_DOCSTRING("Creates an iterator over the array.\n"
                                           "\n"
                                           "Returns:\n"
                                           "    An iterator over the array.\n"))
        .def(PYUNREALSDK_STUBGEN_METHOD("__reversed__", "Iterator[T]"), &impl::array_py_reversed,
             // Keep the array alive as long as the iterator is
             py::keep_alive<0, 1>(),
             PYUNREALSDK_STUBGEN_DOCSTRING("Creates a reverse iterator over the array.\n"
                                           "\n"
                                           "Returns:\n"
                                           "    A reverse iterator over the array.\n"))
        .def(PYUNREALSDK_STUBGEN_METHOD("__contains__", "bool"), &impl::array_py_contains,
             PYUNREALSDK_STUBGEN_DOCSTRING("Checks if a value exists in the array.\n"
                                           "\n"
                                           "Args:\n"
                                           "    value: The value to search for.\n"
                                           "Returns:\n"
                                           "    True if the value exists in the array.\n"),
             PYUNREALSDK_STUBGEN_ARG("value"_a, "T", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("__add__", "list[T]"), &impl::array_py_add,
             py::is_operator(),
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Creates a list holding a copy of the array, and extends it with all the values\n"
                 "in the given sequence.\n"
                 "\n"
                 "Args:\n"
                 "    values: The sequence of values to append.\n"),
             PYUNREALSDK_STUBGEN_ARG("values"_a, "Sequence[T]", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("__radd__", "list[T]"), &impl::array_py_radd,
             py::is_operator(),
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Creates a list holding a copy of the array, and extends it with all the values\n"
                 "in the given sequence.\n"
                 "\n"
                 "Args:\n"
                 "    values: The sequence of values to append.\n"),
             PYUNREALSDK_STUBGEN_ARG("values"_a, "Sequence[T]", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("__iadd__", "Self"), &impl::array_py_iadd,
             py::is_operator(),
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Extends the array with all the values in the given sequence in place.\n"
                 "\n"
                 "Args:\n"
                 "    values: The sequence of values to append.\n"),
             PYUNREALSDK_STUBGEN_ARG("values"_a, "Sequence[T]", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("__mul__", "list[T]"), &impl::array_py_mul,
             py::is_operator(),
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Creates a list holding a copy of the array, and repeats all values in it the\n"
                 "given number of times.\n"
                 "\n"
                 "Args:\n"
                 "    num: The number of times to repeat.\n"),
             PYUNREALSDK_STUBGEN_ARG("num"_a, "int", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("__rmul__", "list[T]"), &impl::array_py_mul,
             py::is_operator(),
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Creates a list holding a copy of the array, and repeats all values in it the\n"
                 "given number of times.\n"
                 "\n"
                 "Args:\n"
                 "    num: The number of times to repeat.\n"),
             PYUNREALSDK_STUBGEN_ARG("num"_a, "int", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("__imul__", "Self"), &impl::array_py_imul,
             py::is_operator(),
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Modifies this array in place, repeating all values the given number of times.\n"
                 "\n"
                 "Args:\n"
                 "    num: The number of times to repeat.\n"),
             PYUNREALSDK_STUBGEN_ARG("num"_a, "int", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("append", "None"), &impl::array_py_append,
             PYUNREALSDK_STUBGEN_DOCSTRING("Appends a value to the end of the array.\n"
                                           "\n"
                                           "Args:\n"
                                           "    value: The value to append.\n"),
             PYUNREALSDK_STUBGEN_ARG("value"_a, "T", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("clear", "None"), &impl::array_py_clear,
             PYUNREALSDK_STUBGEN_DOCSTRING("Removes all items from the array."))
        .def(PYUNREALSDK_STUBGEN_METHOD("copy", "list[T]"), &impl::array_py_copy,
             PYUNREALSDK_STUBGEN_DOCSTRING("Creates a list holding a copy of the array."))
        .def(PYUNREALSDK_STUBGEN_METHOD("count", "int"), &impl::array_py_count,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Counts how many of a given value exist in the array.\n"
                 "\n"
                 "Args:\n"
                 "    value: The value to search for.\n"
                 "Returns:\n"
                 "    The number of times the value appears in the array.\n"),
             PYUNREALSDK_STUBGEN_ARG("value"_a, "T", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("extend", "None"), &impl::array_py_extend,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Extends the array with all the values in the given sequence.\n"
                 "\n"
                 "Args:\n"
                 "    values: The sequence of values to append.\n"),
             PYUNREALSDK_STUBGEN_ARG("values"_a, "Sequence[T]", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("index", "int"), &impl::array_py_index,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Finds the first index of the given value in the array.\n"
                 "\n"
                 "Raises ValueError if the value is not present.\n"
                 "\n"
                 "Args:\n"
                 "    value: The value to search for.\n"
                 "    start: The index to start searching for. Defaults to 0.\n"
                 "    stop: The index to stop searching before. Defaults to the end of the array.\n"
                 "Returns:\n"
                 "    The first index of the value in the array.\n"),
             PYUNREALSDK_STUBGEN_ARG("value"_a, "T", ),
             PYUNREALSDK_STUBGEN_ARG("start"_a, "int", "0") = 0,
             PYUNREALSDK_STUBGEN_ARG("stop"_a, "int", "sys.maxsize") =
                 std::numeric_limits<py::ssize_t>::max())
        .def(
            PYUNREALSDK_STUBGEN_METHOD("insert", "None"), &impl::array_py_insert,
            PYUNREALSDK_STUBGEN_DOCSTRING("Inserts an item into the array before the given index.\n"
                                          "\n"
                                          "Args:\n"
                                          "    idx: The index to insert before.\n"
                                          "    value: The value to insert.\n"),
            PYUNREALSDK_STUBGEN_ARG("idx"_a, "int", ), PYUNREALSDK_STUBGEN_ARG("value"_a, "T", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("pop", "T"), &impl::array_py_pop,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Removes an item from the array, and returns a copy of it.\n"
                 "\n"
                 "Args:\n"
                 "    idx: The index to remove the item from.\n"),
             PYUNREALSDK_STUBGEN_ARG("idx"_a, "int", "-1") = -1)
        .def(PYUNREALSDK_STUBGEN_METHOD("remove", "None"), &impl::array_py_remove,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Finds the first instance of the given value in the array, and removes it.\n"
                 "\n"
                 "Raises ValueError if the value is not present.\n"
                 "\n"
                 "Args:\n"
                 "    value: The value to search for.\n"),
             PYUNREALSDK_STUBGEN_ARG("value"_a, "T", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("reverse", "None"), &impl::array_py_reverse,
             PYUNREALSDK_STUBGEN_DOCSTRING("Reverses the array in place."))
        .def(PYUNREALSDK_STUBGEN_METHOD("sort", "None"), &impl::array_py_sort,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Sorts the array in place.\n"
                 "\n"
                 "Args:\n"
                 "    key: A one-arg function used to extract a comparison key.\n"
                 "    reverse: If true, the list is sorted as if each comparison were reversed.\n"),
             PYUNREALSDK_STUBGEN_KW_ONLY(),
             PYUNREALSDK_STUBGEN_ARG("key"_a, "None | Callable[[T], Any]", "None") = py::none{},
             PYUNREALSDK_STUBGEN_ARG("reverse"_a, "bool", "False") = false)
        .def(PYUNREALSDK_STUBGEN_METHOD("_get_address", "int"), &impl::array_py_getaddress,
             PYUNREALSDK_STUBGEN_DOCSTRING("Gets the address of this array, for debugging.\n"
                                           "\n"
                                           "Returns:\n"
                                           "    This array's address.\n"))
        .def(PYUNREALSDK_STUBGEN_METHOD("emplace_struct", "None"), &impl::array_py_emplace_struct,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "If this is an array of structs, inserts a new struct in place.\n"
                 "\n"
                 "This avoids the extra allocations caused by calling unrealsdk.make_struct().\n"
                 "\n"
                 "Throws a TypeError if this is another type of array.\n"
                 "\n"
                 "Args:\n"
                 "    idx: The index to insert before. Defaults to the end of the array.\n"
                 "    *args: Fields on the struct to initialize. Note you must explicitly specify\n"
                 "           idx to use these.\n"
                 "    **kwargs: Fields on the struct to initialize.\n"),
             PYUNREALSDK_STUBGEN_ARG("idx"_a, "int", "sys.maxsize") =
                 std::numeric_limits<py::ssize_t>::max(),
             PYUNREALSDK_STUBGEN_POS_ONLY()                /* alignment */
             PYUNREALSDK_STUBGEN_ARG_N("*args"_a, "Any", ) /* alignment */
             PYUNREALSDK_STUBGEN_ARG_N("**kwargs"_a, "Any", ))
        .def_readwrite(PYUNREALSDK_STUBGEN_ATTR("_type", "ZProperty"), &WrappedArray::type);

    // Create as a class method, see pybind11#1693
    cls.attr(PYUNREALSDK_STUBGEN_CLASSMETHOD("__class_getitem__", "GenericAlias")) =
        py::reinterpret_borrow<py::object>(PyClassMethod_New(
            py::cpp_function(
                &impl::array_py_class_getitem,
                PYUNREALSDK_STUBGEN_DOCSTRING(
                    "No-op, implemented to allow type stubs to treat this as a generic type.\n"
                    "\n"
                    "Args:\n"
                    "    *args: Ignored.\n"
                    "    **kwargs: Ignored.\n"
                    "Returns:\n"
                    "    The WrappedArray class.\n"),
                "cls"_a)
                .ptr()));
    PYUNREALSDK_STUBGEN_ARG_N("*args"_a, "Any", )
    PYUNREALSDK_STUBGEN_ARG_N("**kwargs"_a, "Any", )
}

}  // namespace pyunrealsdk::unreal

#endif
