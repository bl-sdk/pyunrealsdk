#include "pyunrealsdk/unreal_bindings/wrapped_array_impl.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/format.h"
#include "unrealsdk/unreal/cast_prop.h"
#include "unrealsdk/unreal/prop_traits.h"
#include "unrealsdk/unreal/wrappers/unreal_pointer.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal::impl {

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

}  // namespace

// =================================================================================================

void array_py_new_init(const py::args& /*args*/, const py::kwargs& /*kwargs*/) {
    throw py::type_error("Cannot create new instances of wrapped arrays.");
}

std::string array_py_repr(const WrappedArray& self) {
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
}

py::object array_py_getitem(const WrappedArray& self, Py_ssize_t idx) {
    return array_get(self, convert_py_idx(self, idx));
}

void array_py_setitem(WrappedArray& self, Py_ssize_t idx, const py::object& value) {
    return array_set(self, convert_py_idx(self, idx), value);
}

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

py::iterator array_py_iter(const WrappedArray& self) {
    return py::make_iterator(ArrayIterator::begin(self), ArrayIterator::end(self));
}

py::iterator array_py_reversed(const WrappedArray& self) {
    return py::make_iterator(std::make_reverse_iterator(ArrayIterator::end(self)),
                             std::make_reverse_iterator(ArrayIterator::begin(self)));
}

bool array_py_contains(const WrappedArray& self, const py::object& value) {
    return std::find_if(ArrayIterator::begin(self), ArrayIterator::end(self),
                        [&value](auto other) { return value.equal(other); })
           != ArrayIterator::end(self);
}

py::list array_py_add(WrappedArray& self, const py::sequence& other) {
    return array_py_copy(self) + other;
}

WrappedArray& array_py_iadd(WrappedArray& self, const py::sequence& other) {
    array_py_extend(self, other);
    return self;
};

py::list array_py_mul(WrappedArray& self, Py_ssize_t other) {
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
}

void array_py_imul(WrappedArray& self, Py_ssize_t other) {
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
}

void array_py_append(WrappedArray& self, const py::object& value) {
    array_validate_value(self, value);

    auto size = self.size();
    self.resize(size + 1);
    array_set(self, size, value);
}

void array_py_clear(WrappedArray& self) {
    for (size_t i = 0; i < self.size(); i++) {
        array_destroy(self, i);
    }
    self.resize(0);
}

size_t array_py_count(const WrappedArray& self, const py::object& value) {
    return std::count_if(ArrayIterator::begin(self), ArrayIterator::end(self),
                         [&value](auto other) { return value.equal(other); });
}

py::list array_py_copy(WrappedArray& self) {
    py::list list{};
    for (size_t i = 0; i < self.size(); i++) {
        list.append(array_get(self, i));
    }
    return list;
}

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

size_t array_py_index(const WrappedArray& self,
                      const py::object& value,
                      Py_ssize_t start,
                      Py_ssize_t stop) {
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

void array_py_insert(WrappedArray& self, Py_ssize_t py_idx, const py::object& value) {
    array_validate_value(self, value);
    auto idx = convert_py_idx(self, py_idx);
    auto size = self.size();

    // Don't move if appending
    if (idx != size) {
        auto data = reinterpret_cast<uintptr_t>(self.base->data);
        auto element_size = self.type->ElementSize;

        auto src = data + (idx * element_size);
        auto remaining_size = (size - idx) * element_size;
        memmove(reinterpret_cast<void*>(src + element_size), reinterpret_cast<void*>(src),
                remaining_size);
    }

    self.resize(size + 1);
    array_set(self, idx, value);
}

py::object array_py_pop(WrappedArray& self, Py_ssize_t py_idx) {
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
}

void array_py_remove(WrappedArray& self, const py::object& value) {
    array_py_delitem(self, static_cast<Py_ssize_t>(array_py_index(self, value)));
}

void array_py_reverse(WrappedArray& self, const py::object& value) {
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
}

}  // namespace pyunrealsdk::unreal::impl
