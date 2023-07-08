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
size_t convert_py_idx(const WrappedArray& arr, py::ssize_t idx) {
    auto size = static_cast<py::ssize_t>(arr.size());
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
 * @brief Deletes a range of entries of arbitrary type in an array.
 *
 * @param arr The array to modify.
 * @param start The start of the range to delete, inclusive.
 * @param stop The end of the range to delete, exclusive.
 */
void array_delete_range(WrappedArray& arr, size_t start, size_t stop) {
    cast_prop(arr.type, [&]<typename T>(const T* /*prop*/) {
        for (auto idx = start; idx < stop; idx++) {
            arr.destroy_at<T>(idx);
        }
    });

    // Don't move if deleting the end of the array
    auto size = arr.size();
    auto num_deleted = stop - start;

    if (stop != size) {
        auto data = reinterpret_cast<uintptr_t>(arr.base->data);
        auto element_size = arr.type->ElementSize;

        auto dest = data + (start * element_size);
        auto src = dest + (num_deleted * element_size);
        auto remaining_size = (size - stop) * element_size;
        memmove(reinterpret_cast<void*>(dest), reinterpret_cast<void*>(src), remaining_size);
    }

    arr.resize(size - num_deleted);
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

py::object array_py_getitem(const WrappedArray& self, const py::object& py_idx) {
    if (py::isinstance<py::ssize_t>(py_idx)) {
        return array_get(self, convert_py_idx(self, py::cast<py::ssize_t>(py_idx)));
    }
    if (!py::isinstance<py::slice>(py_idx)) {
        std::string idx_type_name = py::str(py::type::of(py_idx).attr("__name__"));
        throw py::type_error(unrealsdk::fmt::format(
            "array indices must be integers or slices, not {}", idx_type_name));
    }
    auto slice = py::cast<py::slice>(py_idx);

    py::ssize_t start = 0;
    py::ssize_t stop = 0;
    py::ssize_t step = 0;
    py::ssize_t slicelength = 0;
    if (!slice.compute(static_cast<py::ssize_t>(self.size()), &start, &stop, &step, &slicelength)) {
        throw py::error_already_set();
    }

    py::list ret{slicelength};
    for (auto i = 0; i < slicelength; i++) {
        ret[i] = array_get(self, i);
        start += step;
    }
    return ret;
}

void array_py_setitem(WrappedArray& self, const py::object& py_idx, const py::object& value) {
    if (py::isinstance<py::ssize_t>(py_idx)) {
        array_set(self, convert_py_idx(self, py::cast<py::ssize_t>(py_idx)), value);
        return;
    }
    if (!py::isinstance<py::slice>(py_idx)) {
        std::string idx_type_name = py::str(py::type::of(py_idx).attr("__name__"));
        throw py::type_error(unrealsdk::fmt::format(
            "array indices must be integers or slices, not {}", idx_type_name));
    }
    if (!py::isinstance<py::sequence>(value)) {
        throw py::type_error("can only assign a sequence");
    }

    auto slice = py::cast<py::slice>(py_idx);
    auto value_seq = py::cast<py::sequence>(value);
    auto values_size = static_cast<py::ssize_t>(value_seq.size());

    py::ssize_t start = 0;
    py::ssize_t stop = 0;
    py::ssize_t step = 0;
    py::ssize_t slicelength = 0;
    if (!slice.compute(static_cast<py::ssize_t>(self.size()), &start, &stop, &step, &slicelength)) {
        throw py::error_already_set();
    }

    // If we have 1-1 replacements
    if (slicelength == values_size) {
        // Allow arbitrary steps
        for (auto i = 0; i < slicelength; i++) {
            array_set(self, start, value_seq[i]);
            start += step;
        }
        return;
    }

    // Otherwise, if sizes don't match, we must not be doing an extended slice, it needs to be
    // continuous
    // This logic sounds backwards, but it lets our simpler code early exit, and it works the same
    // way as list
    if (step != 1 && step != -1) {
        throw py::value_error(unrealsdk::fmt::format(
            "attempt to assign sequence of size {} to extended slice of size {}", value_seq.size(),
            slicelength));
    }

    if (step < 0) {
        auto tmp = start;
        start = stop;
        stop = tmp;
        step *= -1;
    }

    auto new_size = self.size() + values_size - slicelength;
    if (new_size > self.capacity()) {
        self.reserve(new_size);
    }

    // As much as I'd love to memmove the data buffer here, we can't because we don't know that the
    // values in our sequence are valid types.
    // Instead we need to stick to the basic operations, such that an exception at any point leaves
    // the array in a valid state (even if it's not what was intended).
    // Chosing to do this by deleting all overwritten objects, then inserting all the new ones.
    array_py_delitem(self, py::slice(start, stop, 1));

    for (auto value_idx = 0; value_idx < values_size; value_idx++) {
        array_py_insert(self, start + value_idx, value_seq[value_idx]);
    }
}

void array_py_delitem(WrappedArray& self, const py::object& py_idx) {
    py::ssize_t start = 0;
    py::ssize_t stop = 0;
    py::ssize_t step = 0;
    py::ssize_t slicelength = 0;

    if (py::isinstance<py::ssize_t>(py_idx)) {
        start = static_cast<py::ssize_t>(convert_py_idx(self, py::cast<py::ssize_t>(py_idx)));
        stop = start + 1;
        step = 1;
        slicelength = 1;
    } else if (py::isinstance<py::slice>(py_idx)) {
        auto slice = py::cast<py::slice>(py_idx);
        if (!slice.compute(static_cast<py::ssize_t>(self.size()), &start, &stop, &step,
                           &slicelength)) {
            throw py::error_already_set();
        }
    } else {
        std::string idx_type_name = py::str(py::type::of(py_idx).attr("__name__"));
        throw py::type_error(unrealsdk::fmt::format(
            "array indices must be integers or slices, not {}", idx_type_name));
    }

    // If we don't have continuous ranges
    if (step != 1 && step != -1) {
        // Delete each index individually
        for (auto i = 0; i < slicelength; i++) {
            array_delete_range(self, start, start + 1);
            start += step;
        }
    } else {
        // Otherwise, we can delete everything in one go
        if (step < 0) {
            auto tmp = start;
            start = stop;
            stop = tmp;
        }
        array_delete_range(self, start, stop);
    }
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

py::list array_py_mul(WrappedArray& self, py::ssize_t other) {
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

WrappedArray& array_py_imul(WrappedArray& self, py::ssize_t other) {
    if (other == 1) {
        return self;
    }
    if (other < 1) {
        array_py_clear(self);
        return self;
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

    return self;
}

void array_py_append(WrappedArray& self, const py::object& value) {
    array_validate_value(self, value);

    auto size = self.size();
    self.resize(size + 1);
    array_set(self, size, value);
}

void array_py_clear(WrappedArray& self) {
    array_delete_range(self, 0, self.size());
    self.resize(0);
}

size_t array_py_count(const WrappedArray& self, const py::object& value) {
    return std::count_if(ArrayIterator::begin(self), ArrayIterator::end(self),
                         [&value](auto other) { return value.equal(other); });
}

py::list array_py_copy(WrappedArray& self) {
    auto size = self.size();
    py::list list{size};
    for (size_t i = 0; i < size; i++) {
        list[i] = array_get(self, i);
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
                      py::ssize_t start,
                      py::ssize_t stop) {
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

void array_py_insert(WrappedArray& self, py::ssize_t py_idx, const py::object& value) {
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

py::object array_py_pop(WrappedArray& self, py::ssize_t py_idx) {
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

    array_delete_range(self, idx, idx + 1);

    return ret;
}

void array_py_remove(WrappedArray& self, const py::object& value) {
    auto idx = array_py_index(self, value);
    array_delete_range(self, idx, idx + 1);
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
