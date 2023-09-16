#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/static_py_object.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#if PY_VERSION_HEX >= 0x030D0000
#define PY_IS_FINALIZING() Py_IsFinalizing()
#else
#define PY_IS_FINALIZING() _Py_IsFinalizing()
#endif
// NOLINTEND(cppcoreguidelines-macro-usage)

namespace pyunrealsdk {

StaticPyObject::StaticPyObject() : inner_obj(py::reinterpret_steal<py::object>(nullptr)) {}

StaticPyObject::StaticPyObject(const py::object& obj) : inner_obj(obj) {}
StaticPyObject::StaticPyObject(py::object&& obj) noexcept : inner_obj(std::move(obj)) {}

StaticPyObject::StaticPyObject(const StaticPyObject& obj) = default;
StaticPyObject::StaticPyObject(StaticPyObject&& obj) noexcept = default;

StaticPyObject::~StaticPyObject() {
    // Release just moves some pointers around, it's completely safe
    auto handle = this->inner_obj.release();

    // If finialized, we have to assume it's an invalid pointer which Python already freed
    // Trying to grab the GIL can cause the thread to terminate if finalizing, so also check that
    if (PY_IS_FINALIZING() != 0 || Py_IsInitialized() == 0) {
        return;
    }

    const py::gil_scoped_acquire gil{};
    if (handle) {
        handle.dec_ref();
    }
}

StaticPyObject& StaticPyObject::operator=(const py::object& obj) {
    this->inner_obj = obj;
    return *this;
}
StaticPyObject& StaticPyObject::operator=(py::object&& obj) noexcept {
    this->inner_obj = std::move(obj);
    return *this;
}
StaticPyObject& StaticPyObject::operator=(const StaticPyObject& obj) = default;
StaticPyObject& StaticPyObject::operator=(StaticPyObject&& obj) noexcept = default;

StaticPyObject::operator py::object() const {
    return this->inner_obj;
}
py::object& StaticPyObject::obj(void) {
    return this->inner_obj;
}

StaticPyObject::operator bool() const {
    return (bool)this->inner_obj;
}
}  // namespace pyunrealsdk
