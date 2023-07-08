#include "pyunrealsdk/unreal_bindings/wrapped_array.h"
#include <pybind11/detail/common.h>
#include <pybind11/pytypes.h>
#include <algorithm>
#include <optional>
#include "pyconfig.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/format.h"
#include "unrealsdk/unreal/cast_prop.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"
#include "unrealsdk/utils.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

namespace {

/**
 * @brief Converts a python style index (which may be negative) to an absolute one.
 * @note Throws index error if out of range.
 *
 * @param arr The array to index.
 * @param idx The python index.
 * @return The equivalent absolute index.
 */
size_t convert_py_idx(const WrappedArray& arr, Py_ssize_t idx) {
    auto size = static_cast<Py_ssize_t>(arr.size());
    if (idx < 0) {
        idx += size;
    }
    if (0 > idx || idx >= size) {
        throw py::index_error("array index out of range");
    }
    return idx;
}

/**
 * @brief Gets an entry from an array as a a python object.
 *
 * @param arr The array to get from.
 * @param idx The index to get.
 * @return The value in the array as a python object.
 */
py::object array_get(const WrappedArray& arr, size_t idx) {
    py::object ret{};
    cast_prop(arr.type, [&]<typename T>(const T* /*prop*/) { ret = py::cast(arr.get_at<T>(idx)); });

    return ret;
}

/**
 * @brief Sets an entry in an array from a python object.
 *
 * @param arr The array to modify.
 * @param idx The index to set.
 * @param value The value to set.
 */
void array_set(WrappedArray& arr, size_t idx, const py::object& value) {
    cast_prop(arr.type, [&]<typename T>(const T* /*prop*/) {
        arr.set_at<T>(idx, py::cast<typename PropTraits<T>::Value>(value));
    });
}

/**
 * @brief Ensures a value is compatible with the given array.
 * @note Intended to be used when there's extra, non-reversible, work to do before it's possible to
 *       call array_set.
 *
 * @param arr The array to check against.
 * @param value The value to check.
 */
void array_validate_value(const WrappedArray& arr, const py::object& value) {
    cast_prop(arr.type, [&]<typename T>(const T* /*prop*/) {
        py::cast<typename PropTraits<T>::Value>(value);
    });
}

/**
 * @brief Destroys an entry of arbitrary type in an array.
 * @note Does not move surrounding entries.
 *
 * @param arr The array to modify.
 * @param idx The index to destory.
 */
void array_destroy(WrappedArray& arr, size_t idx) {
    cast_prop(arr.type, [&]<typename T>(const T* /*prop*/) { arr.destroy_at<T>(idx); });
}

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
     * @param idx The idnex to start iterating at.
     */
    ArrayIterator(const WrappedArray& arr, size_t idx)
        : arr(&arr), idx(idx >= arr.size() ? std::numeric_limits<size_t>::max() : idx) {}

    /**
     * @brief Gets the python object this iterator is pointing at.
     *
     * @return The python object we're pointing at.
     */
    reference operator*() const { return array_get(*this->arr, this->idx); }

    /**
     * @brief Moves the iterator.
     *
     * @return A reference to this iterator before/after the move, as relevant.
     */
    ArrayIterator& operator++() {
        if (++this->idx >= this->arr->size()) {
            this->idx = std::numeric_limits<size_t>::max();
        }
        return *this;
    }
    ArrayIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
    ArrayIterator& operator--() {
        if (this->idx == std::numeric_limits<size_t>::max()) {
            this->idx = this->arr->size() - 1;
        } else {
            this->idx--;
        }
        return *this;
    }
    ArrayIterator operator--(int) {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    /**
     * @brief Checks if the iterator is pointing at the same location as another.
     *
     * @param rhs The other iterator.
     * @return True if the iterator is/isn't pointing at the same location.
     */
    bool operator==(const ArrayIterator& rhs) const { return this->idx == rhs.idx; }
    bool operator!=(const ArrayIterator& rhs) const { return !(*this == rhs); };

    /**
     * @brief Helper to create iterators at the beginning/end of an array.
     *
     * @param arr The array to create the iterator on.
     * @return The new iterator.
     */
    static ArrayIterator begin(const WrappedArray& arr) { return {arr, 0}; }
    static ArrayIterator end(const WrappedArray& arr) {
        return {arr, std::numeric_limits<size_t>::max()};
    }
};

/**
 * @brief Implements WrappedArray.__delitem__().
 *
 * @param self The array to delete an index of.
 * @param py_idx The python index to delete at.
 */
void array_py_delitem(WrappedArray& self, Py_ssize_t py_idx) {
    auto idx = convert_py_idx(self, py_idx);
    array_destroy(self, idx);

    // Don't move if deleting the end of the array
    auto size = self.size();
    if (idx != (size - 1)) {
        auto data = reinterpret_cast<uintptr_t>(self.base->data);
        auto element_size = self.type->ElementSize;

        auto dest = data + (idx * element_size);
        auto remaining_size = (size - idx) * element_size;
        memmove(reinterpret_cast<void*>(dest), reinterpret_cast<void*>(dest + element_size),
                remaining_size);
    }

    self.resize(size - 1);
}

/**
 * @brief Implements WrappedArray.clear().
 *
 * @param self The array to clear.
 */
void array_py_clear(WrappedArray& self) {
    for (size_t i = 0; i < self.size(); i++) {
        array_destroy(self, i);
    }
    self.resize(0);
}

/**
 * @brief Implements WrappedArray.copy().
 *
 * @param self The array to copy.
 * @return A list holding a copy of the array.
 */
py::list array_py_copy(WrappedArray& self) {
    py::list list{};
    for (size_t i = 0; i < self.size(); i++) {
        list.append(array_get(self, i));
    }
    return list;
}

/**
 * @brief Implements WrappedArray.extend().
 *
 * @param self The array to extend.
 * @param values The sequence of values to append.
 */
void array_py_extend(WrappedArray& self, const py::sequence& values) {
    auto idx = self.size();

    self.reserve(idx + values.size());

    for (const auto& val : values) {
        // Set beyond the end of the array - we know the memory's reserved.
        // if we throw, the size won't have increased.
        array_set(self, idx, val);

        // Since we previously reserved memory, this shouldn't cause re-allocations.
        self.resize(++idx);
    }
}

/**
 * @brief Implements WrappedArray.index().
 *
 * @param self The array to look through.
 * @param value The value to look for.
 * @param start The index to start searching at.
 * @param stop The index to stop searching at.
 * @return The index of the given value.
 */
size_t array_py_index(const WrappedArray& self,
                      const py::object& value,
                      Py_ssize_t start = 0,
                      Py_ssize_t stop = -1) {
    array_validate_value(self, value);

    auto end = ArrayIterator{self, convert_py_idx(self, stop)};

    auto location = std::find_if(ArrayIterator{self, convert_py_idx(self, start)}, end,
                                 [&value](auto other) { return value.equal(other); });
    if (location == end) {
        throw py::value_error(
            unrealsdk::fmt::format("{} is not in array", std::string(py::str(value))));
    }
    return location.idx;
}

}  // namespace

void register_wrapped_array(py::module_& mod) {
    // There are a lot of methods you need to fully implement a mutable sequence

    // `collections.abc.MutableSequence` simplifies it down to just 5 - but there are three issues:
    // 1. It's mixins are python, which is slow
    // 2. It's mixins are inefficent in places - e.g. extend just repeatedly calls append, which is
    //    multiple new allocations
    // 3. Pybind doesn't really support inheriting from a python type

    // Instead, we implement everything outselves
    // https://docs.python.org/3/reference/datamodel.html#emulating-container-types

    // In addition to the suggested methods, we also implement `copy()` and `clear()`, for more
    // parity with `list`

    // The one suggested method we skip is `sort()` - since most of the property types aren't even
    // sortable to begin with, it's just kind of awkward to implement on our end. At best, we could
    // call `sorted(self, ...)`, and then re-assign everything in the new order - but that's not
    // really much better than doing it in pure python, our C++ functions are supposed to be fast,
    // prefer that cost to be explict to the user.

    py::class_<WrappedArray>(mod, "WrappedArray", "An unreal array wrapper.")
        .def("__new__",
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of wrapped arrays.");
             })
        .def("__init__",
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of wrapped arrays.");
             })
        .def(
            "__repr__",
            [](const WrappedArray& self) {
                std::ostringstream output;
                output << "[";

                for (size_t i = 0; i < self.size(); i++) {
                    if (i > 0) {
                        output << ", ";
                    }

                    output << py::repr(array_get(self, i));
                }

                output << "]";
                return output.str();
            },
            "Gets a string representation of this struct.\n"
            "\n"
            "Returns:\n"
            "    The string representation.")
        .def(
            "__getitem__",
            [](const WrappedArray& self, Py_ssize_t idx) {
                return array_get(self, convert_py_idx(self, idx));
            },
            "Gets an item from the array.\n"
            "\n"
            "Args:\n"
            "    idx: The index to get.\n"
            "Returns:\n"
            "    The item at the given index.",
            "idx"_a)
        .def(
            "__setitem__",
            [](WrappedArray& self, Py_ssize_t idx, const py::object& value) {
                return array_set(self, convert_py_idx(self, idx), value);
            },
            "Sets an item in the array.\n"
            "\n"
            "Args:\n"
            "    idx: The index to set.\n"
            "    value: The value to set.\n",
            "idx"_a, "value"_a)
        .def("__delitem__", &array_py_delitem,
             "Deletes an item from the array.\n"
             "\n"
             "Args:\n"
             "    idx: The index to delete.\n"
             "idx"_a)
        .def("__len__", &WrappedArray::size,
             "Gets the length of the array.\n"
             "\n"
             "Returns:\n"
             "    The length of the array.")
        .def(
            "__iter__",
            [](const WrappedArray& self) {
                return py::make_iterator(ArrayIterator::begin(self), ArrayIterator::end(self));
            },
            // Keep the array alive as long as the iterator is
            py::keep_alive<0, 1>(),
            "Creates an iterator over the array.\n"
            "\n"
            "Returns:\n"
            "    An iterator over the array.")
        .def(
            "__reversed__",
            [](const WrappedArray& self) {
                return py::make_iterator(std::make_reverse_iterator(ArrayIterator::end(self)),
                                         std::make_reverse_iterator(ArrayIterator::begin(self)));
            },
            // Keep the array alive as long as the iterator is
            py::keep_alive<0, 1>(),
            "Creates a reverse iterator over the array.\n"
            "\n"
            "Returns:\n"
            "    A reverse iterator over the array.")
        .def(
            "__contains__",
            [](const WrappedArray& self, const py::object& value) {
                return std::find_if(ArrayIterator::begin(self), ArrayIterator::end(self),
                                    [&value](auto other) { return value.equal(other); })
                       != ArrayIterator::end(self);
            },
            "Checks if a value exists in the array.\n"
            "\n"
            "Args:\n"
            "    value: The value to search for.\n"
            "Returns:\n"
            "    True if the value exists in the array.",
            "value"_a)
        .def(
            "__add__",
            [](WrappedArray& self, const py::sequence& other) {
                return array_py_copy(self) + other;
            },
            py::is_operator(),
            "Creates a list holding a copy of the array, and extends it with all the values\n"
            "in the given sequence.\n"
            "\n"
            "Args:\n"
            "    values: The sequence of values to append.")
        .def(
            "__radd__",
            [](WrappedArray& self, const py::sequence& other) {
                return array_py_copy(self) + other;
            },
            py::is_operator(),
            "Creates a list holding a copy of the array, and extends it with all the values\n"
            "in the given sequence.\n"
            "\n"
            "Args:\n"
            "    values: The sequence of values to append.")
        .def(
            "__iadd__",
            [](WrappedArray& self, const py::sequence& other) {
                array_py_extend(self, other);
                return self;
            },
            py::is_operator(),
            "Extends the array with all the values in the given sequence in place.\n"
            "\n"
            "Args:\n"
            "    values: The sequence of values to append.")
        .def(
            "__mul__",
            [](WrappedArray& self, Py_ssize_t other) {
                if (other == 1) {
                    return array_py_copy(self);
                }
                if (other < 1) {
                    return py::list{};
                }

                auto template_list = array_py_copy(self);
                py::list ret_list{template_list};
                for (auto i = 1; i < other; i++) {
                    ret_list += template_list;
                }

                return ret_list;
            },
            py::is_operator(),
            "Creates a list holding a copy of the array, and repeats all values in it the\n"
            "given number of times.\n"
            "\n"
            "Args:\n"
            "    values: The sequence of values to append")
        .def(
            "__rmul__",
            [](WrappedArray& self, Py_ssize_t other) {
                if (other == 1) {
                    return array_py_copy(self);
                }
                if (other < 1) {
                    return py::list{};
                }

                auto template_list = array_py_copy(self);
                py::list ret_list{template_list};
                for (auto i = 1; i < other; i++) {
                    ret_list += template_list;
                }

                return ret_list;
            },
            py::is_operator(),
            "Creates a list holding a copy of the array, and repeats all values in it the\n"
            "given number of times.\n"
            "\n"
            "Args:\n"
            "    values: The sequence of values to append")
        .def(
            "__imul__",
            [](WrappedArray& self, Py_ssize_t other) {
                if (other == 1) {
                    return;
                }
                if (other < 1) {
                    array_py_clear(self);
                    return;
                }

                cast_prop(self.type, [&self, other]<typename T>(const T* /*prop*/) {
                    auto size = self.size();
                    self.reserve(size * other);

                    // Loop such that we're always adding elements to the end of the array
                    // If we somehow error in the middle of the doing this, it'll leave us in a
                    // stable state.
                    for (auto repetition = 0; repetition < other; repetition++) {
                        for (size_t base_idx = 0; base_idx < size; base_idx++) {
                            auto copy_idx = (repetition * size) + base_idx;
                            self.set_at<T>(copy_idx, self.get_at<T>(base_idx));

                            self.resize(copy_idx + 1);
                        }
                    }
                });
            },
            py::is_operator(),
            "Modifies this array in place, repeating all values the given number of times.\n"
            "\n"
            "Args:\n"
            "    values: The sequence of values to append")
        .def(
            "append",
            [](WrappedArray& self, const py::object& value) {
                array_validate_value(self, value);

                auto size = self.size();
                self.resize(size + 1);
                array_set(self, size, value);
            },
            "Appends a value to the end of the array.\n"
            "\n"
            "Args:\n"
            "    value: The value to append",
            "value"_a)
        .def("clear", &array_py_clear, "Removes all items from the array.")
        .def("copy", &array_py_copy, "Creates a list holding a copy of the array.")
        .def(
            "count",
            [](const WrappedArray& self, const py::object& value) {
                return std::count_if(ArrayIterator::begin(self), ArrayIterator::end(self),
                                     [&value](auto other) { return value.equal(other); });
            },
            "Counts how many of a given value exist in the array.\n"
            "\n"
            "Args:\n"
            "    value: The value to search for.\n"
            "Returns:\n"
            "    The number of times the value appears in the array.",
            "value"_a)
        .def("extend", &array_py_extend,
             "Extends the array with all the values in the given sequence.\n"
             "\n"
             "Args:\n"
             "    values: The sequence of values to append.",
             "values"_a)
        .def("index", &array_py_index,
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
             "value"_a, "start"_a = 0, "stop"_a = -1)
        .def(
            "insert",
            [](WrappedArray& self, Py_ssize_t py_idx, const py::object& value) {
                array_validate_value(self, value);
                auto idx = convert_py_idx(self, py_idx);
                auto size = self.size();

                // Don't move if appending
                if (idx != size) {
                    auto data = reinterpret_cast<uintptr_t>(self.base->data);
                    auto element_size = self.type->ElementSize;

                    auto src = data + (idx * element_size);
                    auto remaining_size = (size - idx) * element_size;
                    memmove(reinterpret_cast<void*>(src + element_size),
                            reinterpret_cast<void*>(src), remaining_size);
                }

                self.resize(size + 1);
                array_set(self, idx, value);
            },
            "Inserts an item into the array before the given index.\n"
            "\n"
            "Args:\n"
            "    idx: The index to insert before.\n"
            "    value: The value to insert.",
            "idx"_a, "value"_a)
        .def(
            "pop",
            [](WrappedArray& self, Py_ssize_t py_idx) {
                auto idx = convert_py_idx(self, py_idx);

                py::object ret{};
                cast_prop(self.type, [&self, &ret, idx]<typename T>(const T* /*prop*/) {
                    auto val = self.get_at<T>(idx);

                    // Explictly make a copy
                    // Some types (structs) are a reference by default, which will break once we
                    // remove them otherwise
                    typename PropTraits<T>::Value val_copy = val;

                    ret = py::cast(val_copy);
                });

                array_py_delitem(self, py_idx);

                return ret;
            },
            "Removes an item from the array, and returns a copy of it.\n"
            "\n"
            "Args:\n"
            "    idx: The index to remove the item from.",
            "idx"_a = -1)
        .def(
            "remove",
            [](WrappedArray& self, const py::object& value) {
                array_py_delitem(self, static_cast<Py_ssize_t>(array_py_index(self, value)));
            },
            "Finds the first instance of the given value in the array, and removes it.\n"
            "\n"
            "Raises ValueError if the value is not present.\n"
            "\n"
            "Args:\n"
            "    value: The value to search for.\n",
            "value"_a)
        .def(
            "reverse",
            [](WrappedArray& self, const py::object& value) {
                cast_prop(self.type, [&]<typename T>(const T* /*prop*/) {
                    auto size = self.size();
                    for (size_t i = 0; i < (size / 2); i++) {
                        auto upper_idx = size - i - 1;

                        auto lower = self.get_at<T>(i);
                        self.set_at<T>(i, self.get_at<T>(upper_idx));
                        self.set_at<T>(upper_idx, lower);
                    }
                    py::cast<typename PropTraits<T>::Value>(value);
                });
            },
            "Reverses the array in place.");
}
}  // namespace pyunrealsdk::unreal
