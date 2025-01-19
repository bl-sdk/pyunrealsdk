#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "pyunrealsdk/unreal_bindings/wrapped_array.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal::impl {

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

py::object array_get(const WrappedArray& arr, size_t idx) {
    if (arr.type->Offset_Internal != 0) {
        throw std::runtime_error(
            "array inner property has non-zero offset, unsure how to handle, aborting!");
    }
    if (arr.type->ArrayDim != 1) {
        throw std::runtime_error(
            "array inner property is fixed array, unsure how to handle, aborting!");
    }

    // Const cast is slightly naughty, but we know the internals aren't going to modify properties
    return py_getattr(
        const_cast<UProperty*>(arr.type),  // NOLINT(cppcoreguidelines-pro-type-const-cast)
        reinterpret_cast<uintptr_t>(arr.base.get()->data) + (arr.type->ElementSize * idx),
        arr.base);
}

void array_set(WrappedArray& arr, size_t idx, const py::object& value) {
    if (arr.type->Offset_Internal != 0) {
        throw std::runtime_error(
            "array inner property has non-zero offset, unsure how to handle, aborting!");
    }
    if (arr.type->ArrayDim != 1) {
        throw std::runtime_error(
            "array inner property is fixed array, unsure how to handle, aborting!");
    }

    py_setattr_direct(
        const_cast<UProperty*>(arr.type),  // NOLINT(cppcoreguidelines-pro-type-const-cast)
        reinterpret_cast<uintptr_t>(arr.base.get()->data) + (arr.type->ElementSize * idx), value);
}

void array_delete_range(WrappedArray& arr, size_t start, size_t stop) {
    cast(arr.type, [&]<typename T>(const T* /*prop*/) {
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

ArrayIterator::ArrayIterator(const WrappedArray& arr, size_t idx)
    : arr(&arr), idx(idx >= arr.size() ? std::numeric_limits<size_t>::max() : idx) {}

ArrayIterator::reference ArrayIterator::operator*() const {
    return array_get(*this->arr, this->idx);
}

ArrayIterator& ArrayIterator::operator++() {
    if (++this->idx >= this->arr->size()) {
        this->idx = std::numeric_limits<size_t>::max();
    }
    return *this;
}
ArrayIterator ArrayIterator::operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
}
ArrayIterator& ArrayIterator::operator--() {
    if (this->idx == std::numeric_limits<size_t>::max()) {
        this->idx = this->arr->size() - 1;
    } else {
        this->idx--;
    }
    return *this;
}
ArrayIterator ArrayIterator::operator--(int) {
    auto tmp = *this;
    --(*this);
    return tmp;
}

bool ArrayIterator::operator==(const ArrayIterator& rhs) const {
    return this->idx == rhs.idx;
}
bool ArrayIterator::operator!=(const ArrayIterator& rhs) const {
    return !(*this == rhs);
};

ArrayIterator ArrayIterator::begin(const WrappedArray& arr) {
    return {arr, 0};
}
ArrayIterator ArrayIterator::end(const WrappedArray& arr) {
    return {arr, std::numeric_limits<size_t>::max()};
}

}  // namespace pyunrealsdk::unreal::impl

#endif
