#ifndef PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_ARRAY_H
#define PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_ARRAY_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace unrealsdk::unreal {

class WrappedArray;

}

namespace pyunrealsdk::unreal {

/**
 * @brief Registers WrappedArray.
 *
 * @param mod The module to register within.
 */
void register_wrapped_array(py::module_& mod);

namespace impl {

using unrealsdk::unreal::WrappedArray;

/**
 * @brief Converts a python style index (which may be negative) to an absolute one.
 * @note Throws index error if out of range.
 *
 * @param arr The array to index.
 * @param idx The python index.
 * @return The equivalent absolute index.
 */
size_t convert_py_idx(const WrappedArray& arr, py::ssize_t idx);

/**
 * @brief Gets an entry from an array as a a python object.
 *
 * @param arr The array to get from.
 * @param idx The index to get.
 * @return The value in the array as a python object.
 */
py::object array_get(const WrappedArray& arr, size_t idx);

/**
 * @brief Sets an entry in an array from a python object.
 *
 * @param arr The array to modify.
 * @param idx The index to set.
 * @param value The value to set.
 */
void array_set(WrappedArray& arr, size_t idx, const py::object& value);

/**
 * @brief Deletes a range of entries of arbitrary type in an array.
 *
 * @param arr The array to modify.
 * @param start The start of the range to delete, inclusive.
 * @param stop The end of the range to delete, exclusive.
 */
void array_delete_range(WrappedArray& arr, size_t start, size_t stop);

/**
 * @brief Iterator over arrays which returns python objects.
 */
struct ArrayIterator {
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = py::object;
    using pointer = py::object;
    using reference = py::object;

    const WrappedArray* arr;
    size_t idx;

    /**
     * @brief Construct a new iterator.
     *
     * @param arr The array to iterate over.
     * @param idx The index to start iterating at.
     */
    ArrayIterator(const WrappedArray& arr, size_t idx);

    /**
     * @brief Gets the python object this iterator is pointing at.
     *
     * @return The python object we're pointing at.
     */
    reference operator*() const;

    /**
     * @brief Moves the iterator.
     *
     * @return A reference to this iterator before/after the move, as relevant.
     */
    ArrayIterator& operator++();
    ArrayIterator operator++(int);
    ArrayIterator& operator--();
    ArrayIterator operator--(int);

    /**
     * @brief Checks if the iterator is pointing at the same location as another.
     *
     * @param rhs The other iterator.
     * @return True if the iterator is/isn't pointing at the same location.
     */
    bool operator==(const ArrayIterator& rhs) const;
    bool operator!=(const ArrayIterator& rhs) const;

    /**
     * @brief Helper to create iterators at the beginning/end of an array.
     *
     * @param arr The array to create the iterator on.
     * @return The new iterator.
     */
    static ArrayIterator begin(const WrappedArray& arr);
    static ArrayIterator end(const WrappedArray& arr);
};

// There are a lot of methods you need to fully implement a mutable sequence.
// `collections.abc.MutableSequence` simplifies it down to just 5 - but there are three issues:
// 1. It's mixins are python, which is slow
// 2. It's mixins are inefficient in places - e.g. extend just repeatedly calls append, which is
//    multiple new allocations
// 3. Pybind doesn't really support inheriting from a python type

// Instead, we implement everything ourselves
// https://docs.python.org/3/reference/datamodel.html#emulating-container-types

// In addition to the suggested methods, we also implement `copy()` and `clear()`, for more parity
// with `list`

// Due to the sheer number of methods we need to implement, we split all the implementations into
// several source files, while the main source file is what registers them and has all the
// documentation

// __new__
void array_py_new(const py::args& args, const py::kwargs& kwargs);
// __init__
WrappedArray* array_py_init(const py::args& args, const py::kwargs& kwargs);
// __repr__
std::string array_py_repr(const WrappedArray& self);
// __getitem__
py::object array_py_getitem(const WrappedArray& self, py::ssize_t py_idx);
py::list array_py_getitem_slice(const WrappedArray& self, const py::slice& slice);
// __setitem__
void array_py_setitem(WrappedArray& self, py::ssize_t py_idx, const py::object& value);
void array_py_setitem_slice(WrappedArray& self, const py::slice& slice, const py::sequence& value);
// __delitem__
void array_py_delitem(WrappedArray& self, py::ssize_t py_idx);
void array_py_delitem_slice(WrappedArray& self, const py::slice& slice);
// __len__
// &WrappedArray::size already works for this
// __iter__
py::iterator array_py_iter(const WrappedArray& self);
// __reversed__
py::iterator array_py_reversed(const WrappedArray& self);
// __contains__
bool array_py_contains(const WrappedArray& self, const py::object& value);
// __add__
py::list array_py_add(WrappedArray& self, const py::sequence& other);
// __radd__
py::list array_py_radd(WrappedArray& self, const py::sequence& other);
// __iadd__
WrappedArray& array_py_iadd(WrappedArray& self, const py::sequence& other);
// __mul__ + __rmul__
py::list array_py_mul(WrappedArray& self, py::ssize_t other);
// __imul__
WrappedArray& array_py_imul(WrappedArray& self, py::ssize_t other);
// __class_getitem__
py::type array_py_class_getitem(const py::type& cls,
                                const py::args& args,
                                const py::kwargs& kwargs);
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
void array_py_reverse(WrappedArray& self);
// sort
void array_py_sort(WrappedArray& self, const py::object& key, bool reverse);
// _get_address
uintptr_t array_py_getaddress(const WrappedArray& self);
// emplace_struct
void array_py_emplace_struct(WrappedArray& self,
                             py::ssize_t py_idx,
                             const py::args& args,
                             const py::kwargs& kwargs);

}  // namespace impl

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_WRAPPED_ARRAY_H */
