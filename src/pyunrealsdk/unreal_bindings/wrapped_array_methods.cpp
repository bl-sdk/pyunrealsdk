#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/static_py_object.h"
#include "pyunrealsdk/unreal_bindings/wrapped_array.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal::impl {

void array_py_append(WrappedArray& self, const py::object& value) {
    array_validate_value(self, value);

    auto size = self.size();
    self.resize(size + 1);
    array_set(self, size, value);
}

void array_py_clear(WrappedArray& self) {
    self.resize(0);
}

size_t array_py_count(const WrappedArray& self, const py::object& value) {
    return std::count_if(ArrayIterator::begin(self), ArrayIterator::end(self),
                         [&value](const auto& other) { return value.equal(other); });
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
        // Set beyond the end of the array - we know the memory's reserved so it's safe.
        // This means we only up the size after having successfully added a new entry.
        try {
            array_set(self, idx, val);
        } catch (...) {
            // On exception, make sure to delete whatever value may have just been set
            array_delete_range(self, idx, idx + 1);
            throw;
        }

        // Since we previously reserved memory, this shouldn't cause re-allocations.
        self.resize(++idx);
    }
}

size_t array_py_index(const WrappedArray& self,
                      const py::object& value,
                      py::ssize_t start,
                      py::ssize_t stop) {
    array_validate_value(self, value);

    // `list.index` method handles indexes a little differently to most methods. Essentially, any
    // index is allowed, and it's just implicitly clamped to the size of the array. You're allowed
    // to do some stupid things like `["a"].index("a", -100, -200)`, it just gives a not in list
    // error.

    // Firstly, wrap negative indexes
    auto size = static_cast<py::ssize_t>(self.size());
    if (start < 0) {
        start += size;
    }
    if (stop < 0) {
        stop += size;
    }

    // Clamp to the start of the array
    start = std::max(start, py::ssize_t{0});
    stop = std::max(stop, py::ssize_t{0});

    // Make sure the start is actually before the stop
    if (start < stop) {
        // If the stop index is beyond the end, this automatically becomes an end of array iterator
        auto end = ArrayIterator{self, static_cast<size_t>(stop)};

        auto location = std::find_if(ArrayIterator{self, static_cast<size_t>(start)}, end,
                                     [&value](const auto& other) { return value.equal(other); });
        if (location != end) {
            return location.idx;
        }
    }

    throw py::value_error(
        unrealsdk::fmt::format("{} is not in array", std::string(py::repr(value))));
}

void array_py_insert(WrappedArray& self, py::ssize_t py_idx, const py::object& value) {
    array_validate_value(self, value);

    auto size = self.size();

    // Allow specifying one past the end, to insert at the end
    // insert(-1) should insert before the last element, so goes through the normal conversion
    auto idx = static_cast<size_t>(py_idx) == size ? py_idx : convert_py_idx(self, py_idx);

    self.resize(size + 1);

    // Don't move if appending
    if (idx != size) {
        auto data = reinterpret_cast<uintptr_t>(self.base->data);
        auto element_size = self.type->ElementSize;

        auto src = data + (idx * element_size);
        auto remaining_size = (size - idx) * element_size;
        memmove(reinterpret_cast<void*>(src + element_size), reinterpret_cast<void*>(src),
                remaining_size);
    }

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
    PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> storage;
    auto& sorted = storage
                       .call_once_and_store_result(
                           []() { return py::module_::import("builtins").attr("sorted"); })
                       .get_stored();

    py::sequence sorted_array = sorted(self, "key"_a = key, "reverse"_a = reverse);

    cast(self.type, [&self, &sorted_array]<typename T>(const T* /*prop*/) {
        auto size = self.size();
        for (size_t i = 0; i < size; i++) {
            auto val = py::cast<typename PropTraits<T>::Value>(sorted_array[i]);
            self.set_at<T>(i, val);
        }
    });
}

uintptr_t array_py_getaddress(const WrappedArray& self) {
    return reinterpret_cast<uintptr_t>(self.base.get());
}

}  // namespace pyunrealsdk::unreal::impl

#endif
