#include "pyunrealsdk/unreal_bindings/wrapped_array.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/unreal/cast_prop.h"
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
        idx = size - idx;
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

struct ArrayIterator {
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = py::object;
    using pointer = py::object;
    using reference = py::object;

    const WrappedArray* arr;
    size_t idx;

    reference operator*() const { return array_get(*this->arr, this->idx); }

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

    bool operator==(const ArrayIterator& rhs) const { return this->idx == rhs.idx; }
    bool operator!=(const ArrayIterator& rhs) const { return !(*this == rhs); };

    static ArrayIterator begin(const WrappedArray& arr) { return {&arr, 0}; }
    static ArrayIterator end(const WrappedArray& arr) {
        return {&arr, std::numeric_limits<size_t>::max()};
    }
};

}  // namespace

void register_wrapped_array(py::module_& mod) {
    // There are a lot of methods you need to fully implement a mutable sequence

    // `collections.abc.MutableSequence` simplifies it down to just 5 - but there are three issues:
    // 1. It's mixins are python, which is slow
    // 2. It's mixins are inefficent in places - e.g. extend just repeatedly calls append, which is
    //    multiple new allocations
    // 3. Pybind doesn't really support inheriting from a python type

    // Instead, we implement everything outselves

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
        .def(
            "__delitem__",
            [](WrappedArray& self, Py_ssize_t py_idx) {
                auto idx = convert_py_idx(self, py_idx);
                array_destroy(self, idx);

                // Don't move if deleting the end of the array
                auto size = self.size();
                if (idx != (size - 1)) {
                    auto data = reinterpret_cast<uintptr_t>(self.base->data);
                    auto element_size = self.type->ElementSize;

                    auto dest = data + (idx * element_size);
                    auto remaining_size = (size - idx) * element_size;
                    memmove(reinterpret_cast<void*>(dest),
                            reinterpret_cast<void*>(dest + element_size), remaining_size);
                }

                self.resize(size - 1);
            },
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
            [](WrappedArray& self) {
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
            [](WrappedArray& self) {
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
            [](WrappedArray& self, const py::object& value) {
                array_validate_value(self, value);
                return std::find_if(ArrayIterator::begin(self), ArrayIterator::end(self),
                                    [&value](auto other) { return value.equal(other); })
                       != ArrayIterator::end(self);
            },
            "Checks if a value exists in the array.\n"
            "\n"
            "Args:\n"
            "    value: The value to search for.\n"
            "Returns:\n"
            "    True if the value exists in the array.")
        .def(
            "insert",
            [](WrappedArray& arr, Py_ssize_t py_idx, const py::object& value) {
                array_validate_value(arr, value);
                auto idx = convert_py_idx(arr, py_idx);
                auto size = arr.size();

                // Don't move if appending
                if (idx != size) {
                    auto data = reinterpret_cast<uintptr_t>(arr.base->data);
                    auto element_size = arr.type->ElementSize;

                    auto src = data + (idx * element_size);
                    auto remaining_size = (size - idx) * element_size;
                    memmove(reinterpret_cast<void*>(src + element_size),
                            reinterpret_cast<void*>(src), remaining_size);
                }

                arr.resize(size + 1);
                array_set(arr, idx, value);
            },
            "Inserts an item into the array before the given index.\n"
            "\n"
            "Args:\n"
            "    idx: The index to insert before.\n"
            "    value: The value to insert.",
            "idx"_a, "value"_a);

    // TODO:
    // append
    // count
    // index
    // extend
    // pop
    // remove
    // reverse
    // sort
    // __add__
    // __radd__
    // __iadd__
    // __mul__
    // __rmul__
    // __imul__
}
}  // namespace pyunrealsdk::unreal
