#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/wrapped_array.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_wrapped_array(py::module_& mod) {
    auto cls = py::class_<WrappedArray>(mod, "WrappedArray");
    cls.def("__new__", &impl::array_py_new)
        .def(py::init(&impl::array_py_init))
        .def("__repr__", &impl::array_py_repr,
             "Gets a string representation of this array.\n"
             "\n"
             "Returns:\n"
             "    The string representation.")
        .def("__getitem__", &impl::array_py_getitem,
             "Gets an item from the array.\n"
             "\n"
             "Args:\n"
             "    idx: The index to get.\n"
             "Returns:\n"
             "    The item at the given index.",
             "idx"_a)
        .def("__getitem__", &impl::array_py_getitem_slice,
             "Gets a range from the array.\n"
             "\n"
             "Args:\n"
             "    range: The range to get.\n"
             "Returns:\n"
             "    The items in the given range.",
             "range"_a)
        .def("__setitem__", &impl::array_py_setitem,
             "Sets an item in the array.\n"
             "\n"
             "Args:\n"
             "    idx: The index to set.\n"
             "    value: The value to set.",
             "idx"_a, "value"_a)
        .def("__setitem__", &impl::array_py_setitem_slice,
             "Sets a range of items in the array.\n"
             "\n"
             "Args:\n"
             "    range: The range to set.\n"
             "    value: The values to set.",
             "range"_a, "value"_a)
        .def("__delitem__", &impl::array_py_delitem,
             "Deletes an item from the array.\n"
             "\n"
             "Args:\n"
             "    idx: The index to delete.",
             "idx"_a)
        .def("__delitem__", &impl::array_py_delitem_slice,
             "Deletes a range from the array.\n"
             "\n"
             "Args:\n"
             "    range: The range to delete.",
             "range"_a)
        .def("__len__", &WrappedArray::size,
             "Gets the length of the array.\n"
             "\n"
             "Returns:\n"
             "    The length of the array.")
        .def("__iter__", &impl::array_py_iter,
             // Keep the array alive as long as the iterator is
             py::keep_alive<0, 1>(),
             "Creates an iterator over the array.\n"
             "\n"
             "Returns:\n"
             "    An iterator over the array.")
        .def("__reversed__", &impl::array_py_reversed,
             // Keep the array alive as long as the iterator is
             py::keep_alive<0, 1>(),
             "Creates a reverse iterator over the array.\n"
             "\n"
             "Returns:\n"
             "    A reverse iterator over the array.")
        .def("__contains__", &impl::array_py_contains,
             "Checks if a value exists in the array.\n"
             "\n"
             "Args:\n"
             "    value: The value to search for.\n"
             "Returns:\n"
             "    True if the value exists in the array.",
             "value"_a)
        .def("__add__", &impl::array_py_add, py::is_operator(),
             "Creates a list holding a copy of the array, and extends it with all the values\n"
             "in the given sequence.\n"
             "\n"
             "Args:\n"
             "    values: The sequence of values to append.",
             "values"_a)
        .def("__radd__", &impl::array_py_radd, py::is_operator(),
             "Creates a list holding a copy of the array, and extends it with all the values\n"
             "in the given sequence.\n"
             "\n"
             "Args:\n"
             "    values: The sequence of values to append.",
             "values"_a)
        .def("__iadd__", &impl::array_py_iadd, py::is_operator(),
             "Extends the array with all the values in the given sequence in place.\n"
             "\n"
             "Args:\n"
             "    values: The sequence of values to append.",
             "values"_a)
        .def("__mul__", &impl::array_py_mul, py::is_operator(),
             "Creates a list holding a copy of the array, and repeats all values in it the\n"
             "given number of times.\n"
             "\n"
             "Args:\n"
             "    num: The number of times to repeat.",
             "num"_a)
        .def("__rmul__", &impl::array_py_mul, py::is_operator(),
             "Creates a list holding a copy of the array, and repeats all values in it the\n"
             "given number of times.\n"
             "\n"
             "Args:\n"
             "    num: The number of times to repeat.",
             "num"_a)
        .def("__imul__", &impl::array_py_imul, py::is_operator(),
             "Modifies this array in place, repeating all values the given number of times.\n"
             "\n"
             "Args:\n"
             "    num: The number of times to repeat.",
             "num"_a)
        .def("append", &impl::array_py_append,
             "Appends a value to the end of the array.\n"
             "\n"
             "Args:\n"
             "    value: The value to append",
             "value"_a)
        .def("clear", &impl::array_py_clear, "Removes all items from the array.")
        .def("copy", &impl::array_py_copy, "Creates a list holding a copy of the array.")
        .def("count", &impl::array_py_count,
             "Counts how many of a given value exist in the array.\n"
             "\n"
             "Args:\n"
             "    value: The value to search for.\n"
             "Returns:\n"
             "    The number of times the value appears in the array.",
             "value"_a)
        .def("extend", &impl::array_py_extend,
             "Extends the array with all the values in the given sequence.\n"
             "\n"
             "Args:\n"
             "    values: The sequence of values to append.",
             "values"_a)
        .def("index", &impl::array_py_index,
             "Finds the first index of the given value in the array.\n"
             "\n"
             "Raises ValueError if the value is not present.\n"
             "\n"
             "Args:\n"
             "    value: The value to search for.\n"
             "    start: The index to start searching for. Defaults to 0.\n"
             "    stop: The index to stop searching before. Defaults to the end of the array.\n"
             "Returns:\n"
             "    The first index of the value in the array.",
             "value"_a, "start"_a = 0, "stop"_a = std::numeric_limits<py::ssize_t>::max())
        .def("insert", &impl::array_py_insert,
             "Inserts an item into the array before the given index.\n"
             "\n"
             "Args:\n"
             "    idx: The index to insert before.\n"
             "    value: The value to insert.",
             "idx"_a, "value"_a)
        .def("pop", &impl::array_py_pop,
             "Removes an item from the array, and returns a copy of it.\n"
             "\n"
             "Args:\n"
             "    idx: The index to remove the item from.",
             "idx"_a = -1)
        .def("remove", &impl::array_py_remove,
             "Finds the first instance of the given value in the array, and removes it.\n"
             "\n"
             "Raises ValueError if the value is not present.\n"
             "\n"
             "Args:\n"
             "    value: The value to search for.\n",
             "value"_a)
        .def("reverse", &impl::array_py_reverse, "Reverses the array in place.")
        .def("sort", &impl::array_py_sort,
             "Sorts the array in place.\n"
             "\n"
             "Args:\n"
             "    key: A one-arg function used to extract a comparison key.\n"
             "    reverse: If true, the list is sorted as if each comparison were reversed.",
             py::kw_only{}, "key"_a = py::none{}, "reverse"_a = false)
        .def("_get_address", &impl::array_py_getaddress,
             "Gets the address of this array, for debugging.\n"
             "\n"
             "Returns:\n"
             "    This array's address.")
        .def_readwrite("_type", &WrappedArray::type);

    // Create as a class method, see pybind11#1693
    cls.attr("__class_getitem__") = py::reinterpret_borrow<py::object>(PyClassMethod_New(
        py::cpp_function(&impl::array_py_class_getitem,
                         "No-op, implemented to allow type stubs to treat this as a generic type.\n"
                         "\n"
                         "Args:\n"
                         "    *args: Ignored.\n"
                         "    **kwargs: Ignored.\n"
                         "Returns:\n"
                         "    The WrappedArray class.",
                         "cls"_a)
            .ptr()));
}

}  // namespace pyunrealsdk::unreal

#endif
