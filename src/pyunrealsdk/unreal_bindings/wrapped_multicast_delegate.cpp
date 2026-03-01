#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/wrapped_multicast_delegate.h"
#include "pyunrealsdk/stubgen.h"
#include "pyunrealsdk/unreal_bindings/bound_function.h"
#include "unrealsdk/unreal/structs/fscriptdelegate.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"
#include "unrealsdk/unreal/wrappers/wrapped_multicast_delegate.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

// We treat this class as a pseudo MutableSet - we only allow one copy of each bound function, and
// don't specify any ordering.
// We don't however implement any of the logical set operators

namespace {

// Create a custom iterator type to convert script delegates to bound functions when iterating
struct DelegateIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = BoundFunction;
    using pointer = void;
    using reference = BoundFunction;

    const TArray<FScriptDelegate>* arr = nullptr;
    size_t idx = 0;

    reference operator*(void) const { return *arr->data[this->idx].as_function(); }

    DelegateIterator& operator++() {
        ++this->idx;

        // If we're on the last index, set the array to null to end the iterator
        if (this->idx >= this->arr->size()) {
            this->arr = nullptr;
        }

        return *this;
    }
    DelegateIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const DelegateIterator& rhs) const {
        return this->arr == rhs.arr && (this->arr == nullptr || this->idx == rhs.idx);
    }
    bool operator!=(const DelegateIterator& rhs) const { return !(*this == rhs); }

    static DelegateIterator begin(const WrappedMulticastDelegate& delegate) {
        if (delegate.base->size() == 0) {
            return end(delegate);
        }
        return {.arr = delegate.base.get(), .idx = 0};
    }
    static DelegateIterator end(const WrappedMulticastDelegate& /*delegate*/) {
        return {.arr = nullptr, .idx = 0};
    }
};

/**
 * @brief Searches through a multicast delegate looking for an entry matching a particular function.
 *
 * @param self The multicast delegate to search through.
 * @param value The function to search for.
 * @return A pointer to the matching entry, or nullptr if no matches.
 */
FScriptDelegate* find_matching_delegate(WrappedMulticastDelegate& self,
                                        const BoundFunction& value) {
    auto begin = &self.base->data[0];
    auto end = &self.base->data[self.base->count];

    auto ptr = std::find_if(begin, end, [&value](FScriptDelegate other) {
        // Do the cheap checks first
        if (value.object != other.get_object() || value.func->Name() != other.func_name) {
            return false;
        }

        // Now check it resolves to the same function, which is a more expensive check
        // Manually iterate through fields to avoid the exceptions if we can't find what we're
        // looking for
        for (auto field : value.object->Class()->fields()) {
            if (field->Name() == other.func_name) {
                // Return immediately on the first name match
                return field == value.func;
            }
        }
        return false;
    });

    return ptr == end ? nullptr : ptr;
}

/**
 * @brief Removes an entry from the given multicast delegate by value.
 *
 * @param self The multicast delegate to remove from.
 * @param value The function to remove.
 * @return True if the function was removed, false if it wasn't found.
 */
bool remove_from_multicast_delegate(WrappedMulticastDelegate& self, const BoundFunction& value) {
    auto ptr = find_matching_delegate(self, value);
    if (ptr == nullptr) {
        return false;
    }

    auto end = &self.base->data[self.base->count];
    memmove(ptr, ptr + 1, (end - ptr) * sizeof(*ptr));

    self.base->resize(self.base->size() - 1);
    return true;
}

// This needs to be a function to avoid an unreachable code warning in `pybind11/detail/init.h` in
// `no_nullptr` when compiling with MSVC.
// I assume this is something to do with inlining optimizations - since we don't actually return, it
// can assume it's nullptr, somehow ignoring the throw here, but not the one later?
// Not thrilled with this as a solution, but it works for now

// __init__
WrappedMulticastDelegate* delegate_init_new(const py::args& /* args */,
                                            const py::kwargs& /* kwargs */) {
    throw py::type_error("Cannot create new wrapped multicast delegate instances.");
}

}  // namespace

void register_wrapped_multicast_delegate(py::module_& mod) {
    PYUNREALSDK_STUBGEN_MODULE_N("unrealsdk.unreal")

    py::classh<WrappedMulticastDelegate>(mod,
                                         PYUNREALSDK_STUBGEN_CLASS("WrappedMulticastDelegate", ))
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD_N("__init__") py::init(&delegate_init_new))
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD("__new__"), &delegate_init_new)
        .def(
            // HACK: include the noqa in the return value. This gives us an extra colon but meh.
            PYUNREALSDK_STUBGEN_METHOD("__call__", "None:  # noqa: D417 "),
            [](WrappedMulticastDelegate& self, const py::args& args, const py::kwargs& kwargs) {
                impl::PyCallInfo info{self.signature, args, kwargs};

                // Release the GIL to avoid a deadlock if ProcessEvent is locking.
                const py::gil_scoped_release gil{};
                self.call(info.params);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Calls all functions bound to this delegate.\n"
                "\n"
                "Args:\n"
                "    The unreal function's args. This has all the same semantics as calling a\n"
                "    BoundFunction.\n")                   /* alignment */
            PYUNREALSDK_STUBGEN_ARG_N("*args"_a, "Any", ) /* alignment */
            PYUNREALSDK_STUBGEN_ARG_N("**kwargs"_a, "Any", ))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__contains__", "bool"),
            [](WrappedMulticastDelegate& self, const BoundFunction& value) {
                return find_matching_delegate(self, value) != nullptr;
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Checks if a function is already bound to this delegate.\n"
                "\n"
                "Args:\n"
                "    value: The function to search for.\n"
                "Returns:\n"
                "    True if the function is already bound.\n"),
            PYUNREALSDK_STUBGEN_ARG("value"_a, "BoundFunction", ))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__iter__", "Iterator[BoundFunction]"),
            [](WrappedMulticastDelegate& self) {
                return py::make_iterator(DelegateIterator::begin(self),
                                         DelegateIterator::end(self));
            },
            // Keep the delegate alive as long as the iterator is
            py::keep_alive<0, 1>(),
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Creates an iterator over all functions bound to this delegate.\n"
                "\n"
                "Returns:\n"
                "    An iterator over all functions bound to this delegate.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__len__", "int"),
            [](WrappedMulticastDelegate& self) { return self.base->size(); },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Gets the number of functions which are bound to this delegate.\n"
                "\n"
                "Returns:\n"
                "    The number of bound functions.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__repr__", "str"),
            [](WrappedMulticastDelegate& self) {
                if (self.base->size() == 0) {
                    return std::string{"WrappedMulticastDelegate()"};
                }

                std::ostringstream output;
                output << "{";

                for (size_t i = 0; i < self.base->size(); i++) {
                    if (i > 0) {
                        output << ", ";
                    }
                    output << py::repr(py::cast(self.base->data[i].as_function()));
                }

                output << "}";
                return output.str();
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets a string representation of this delegate.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    The string representation.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("add", "None"),
            [](WrappedMulticastDelegate& self, const BoundFunction& value) {
                if (find_matching_delegate(self, value) == nullptr) {
                    self.push_back(value);
                }
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Binds a new function to this delegate.\n"
                                          "\n"
                                          "This has no effect if the function is already present.\n"
                                          "\n"
                                          "Args:\n"
                                          "    value: The function to bind.\n"),
            PYUNREALSDK_STUBGEN_ARG("value"_a, "BoundFunction", ))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("clear", "None"),
            [](WrappedMulticastDelegate& self) { self.clear(); },
            PYUNREALSDK_STUBGEN_DOCSTRING("Removes all functions bound to this delegate."))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("discard", "None"),
            [](WrappedMulticastDelegate& self, const BoundFunction& value) {
                remove_from_multicast_delegate(self, value);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Removes a function from this delegate if it is present.\n"
                "\n"
                "Args:\n"
                "    value: The function to remove.\n"),
            PYUNREALSDK_STUBGEN_ARG("value"_a, "BoundFunction", ))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("pop", "BoundFunction"),
            [](WrappedMulticastDelegate& self) {
                if (self.base->size() == 0) {
                    throw py::key_error("pop from an empty delegate");
                }

                // The easiest arbitrary element is the one at the end
                auto size = self.base->size();
                auto value = self.base->data[size - 1].as_function();
                self.base->resize(size - 1);

                return value;
            },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Removes an arbitrary function from this delegate.\n"
                "\n"
                "Throws a KeyError if the delegate has no bound functions.\n"
                "\n"
                "Returns:\n"
                "    The function which was removed.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("remove", "None"),
            [](WrappedMulticastDelegate& self, const BoundFunction& value) {
                if (!remove_from_multicast_delegate(self, value)) {
                    throw py::key_error(py::repr(py::cast(value)));
                }
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Removes a function from this delegate.\n"
                                          "\n"
                                          "Throws a KeyError if the function is not present.\n"
                                          "\n"
                                          "Args:\n"
                                          "    value: The function to remove.\n"),
            PYUNREALSDK_STUBGEN_ARG("value"_a, "BoundFunction", ))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_get_address", "int"),
            [](const WrappedMulticastDelegate& self) {
                return reinterpret_cast<uintptr_t>(self.base.get());
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets the address of this delegate, for debugging.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    This delegate's address.\n"))
        .def_readwrite(PYUNREALSDK_STUBGEN_ATTR("_signature", "UFunction"),
                       &WrappedMulticastDelegate::signature);
}

}  // namespace pyunrealsdk::unreal

#endif
