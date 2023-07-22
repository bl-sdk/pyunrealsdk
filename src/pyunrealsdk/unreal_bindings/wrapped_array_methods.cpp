#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/wrapped_array.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal::impl {

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
            unrealsdk::fmt::format("{} is not in array", std::string(py::repr(value))));
    }
    return location.idx;
}

void array_py_insert(WrappedArray& self, py::ssize_t py_idx, const py::object& value) {
    array_validate_value(self, value);

    auto size = self.size();

    // Allow specifying one past the end, to insert at the end
    // insert(-1) should insert before the last element, so goes through the normal conversion
    auto idx = static_cast<size_t>(py_idx) == size ? py_idx : convert_py_idx(self, py_idx);

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
    cast(self.type, [&self, &ret, idx]<typename T>(const T* /*prop*/) {
        auto val = self.get_at<T>(idx);

        // Explicitly make a copy
        // Some types (structs) are a reference by default, which will break once we
        // remove them otherwise
        // NOLINTNEXTLINE(misc-const-correctness)
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

void array_py_reverse(WrappedArray& self) {
    cast(self.type, [&]<typename T>(const T* /*prop*/) {
        auto size = self.size();
        for (size_t i = 0; i < (size / 2); i++) {
            auto upper_idx = size - i - 1;

            auto lower = self.get_at<T>(i);
            self.set_at<T>(i, self.get_at<T>(upper_idx));
            self.set_at<T>(upper_idx, lower);
        }
    });
}

void array_py_sort(WrappedArray& self, const py::object& key, bool reverse) {
    // Implement using the sorted builtin
    // It's just kind of awkward to do from C++, given half our types aren't even sortable to begin
    // with, and we need to be able to compare arbitrary keys anyway
    py::sequence sorted =
        py::module_::import("builtins").attr("sorted")(self, "key"_a = key, "reverse"_a = reverse);

    cast(self.type, [&self, &sorted]<typename T>(const T* /*prop*/) {
        auto size = self.size();
        for (size_t i = 0; i < size; i++) {
            auto val = py::cast<typename PropTraits<T>::Value>(sorted[i]);
            self.set_at<T>(i, val);
        }
    });
}

}  // namespace pyunrealsdk::unreal::impl
