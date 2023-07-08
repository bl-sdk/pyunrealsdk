#ifndef PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_ARRAY_IMPL_H
#define PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_ARRAY_IMPL_H

#include "pyunrealsdk/pch.h"

namespace unrealsdk::unreal {

class WrappedArray;

}

namespace pyunrealsdk::unreal::impl {

using unrealsdk::unreal::WrappedArray;

// There are a lot of methods you need to fully implement a mutable sequence

// `collections.abc.MutableSequence` simplifies it down to just 5 - but there are three issues:
// 1. It's mixins are python, which is slow
// 2. It's mixins are inefficent in places - e.g. extend just repeatedly calls append, which is
//    multiple new allocations
// 3. Pybind doesn't really support inheriting from a python type

// Instead, we implement everything outselves
// https://docs.python.org/3/reference/datamodel.html#emulating-container-types

// In addition to the suggested methods, we also implement `copy()` and `clear()`, for more parity
// with `list`

// The one suggested method we skip is `sort()` - since most of the property types aren't even
// sortable to begin with, it's just kind of awkward to implement on our end. At best, we could call
// `sorted(self, ...)`, and then re-assign everything in the new order - but that's not really much
// better than doing it in pure python, our C++ functions are supposed to be fast, prefer that cost
// to be explict to the user.

// Due to the sheer number of methods we need to implement, this header/source file hold all the
// implementations, while the main source file is what registers them, and has all the documentation

// __new__ + __init__
void array_py_new_init(const py::args& args, const py::kwargs& kwargs);
// __repr__
std::string array_py_repr(const WrappedArray& self);
// __getitem__
py::object array_py_getitem(const WrappedArray& self, const py::object& py_idx);
// __setitem__
void array_py_setitem(WrappedArray& self, const py::object& py_idx, const py::object& value);
// __delitem__
void array_py_delitem(WrappedArray& self, const py::object& py_idx);
// __len__
// &WrappedArray::size already works for this
// __iter__
py::iterator array_py_iter(const WrappedArray& self);
// __reversed__
py::iterator array_py_reversed(const WrappedArray& self);
// __contains__
bool array_py_contains(const WrappedArray& self, const py::object& value);
// __add__ + __radd__
py::list array_py_add(WrappedArray& self, const py::sequence& other);
// __iadd__
WrappedArray& array_py_iadd(WrappedArray& self, const py::sequence& other);
// __mul__ + __rmul__
py::list array_py_mul(WrappedArray& self, py::ssize_t other);
// __imul__
WrappedArray& array_py_imul(WrappedArray& self, py::ssize_t other);
// append
void array_py_append(WrappedArray& self, const py::object& value);
// clear
void array_py_clear(WrappedArray& self);
// count
size_t array_py_count(const WrappedArray& self, const py::object& value);
// copy
py::list array_py_copy(WrappedArray& self);
// extend
void array_py_extend(WrappedArray& self, const py::sequence& values);
// index
size_t array_py_index(const WrappedArray& self,
                      const py::object& value,
                      py::ssize_t start = 0,
                      py::ssize_t stop = -1);
// insert
void array_py_insert(WrappedArray& self, py::ssize_t py_idx, const py::object& value);
// pop
py::object array_py_pop(WrappedArray& self, py::ssize_t py_idx);
// remove
void array_py_remove(WrappedArray& self, const py::object& value);
// reverse
void array_py_reverse(WrappedArray& self, const py::object& value);

}  // namespace pyunrealsdk::unreal::impl

#endif /* PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_ARRAY_IMPL_H */
