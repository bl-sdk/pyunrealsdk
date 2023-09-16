#ifndef PYUNREALSDK_STATIC_PY_OBJECT_H
#define PYUNREALSDK_STATIC_PY_OBJECT_H

#include "pyunrealsdk/pch.h"

namespace pyunrealsdk {

/*
A pybind object wrapper which can safely be stored statically.

The issue with the standard class is it's destructor assumes Python is still running - but when
stored in static memory, it may be called after finalization. Most obviously, this means we can't
grab the GIL, so it will throw an exception during a destructor, and thus crash the game. While this
only happens when you close the game anyway, we don't want the user to see us causing crashes.
*/

class StaticPyObject {
   private:
    py::object inner_obj;

   public:
    /**
     * @brief Construct a new static python object.
     * @note The default constructor is GIL safe, and constructs a null object.
     *
     * @param obj The object to construct based off of.
     */
    StaticPyObject();
    StaticPyObject(const py::object& obj);
    StaticPyObject(py::object&& obj) noexcept;
    StaticPyObject(const StaticPyObject& obj);
    StaticPyObject(StaticPyObject&& obj) noexcept;

    /**
     * @brief Safely destroys this object.
     * @note GIL/Finialization safe.
     */
    ~StaticPyObject();

    /**
     * @brief Assigns to this object.
     *
     * @param obj The object to assign.
     * @return A reference to this object.
     */
    StaticPyObject& operator=(const py::object& obj);
    StaticPyObject& operator=(py::object&& obj) noexcept;
    StaticPyObject& operator=(const StaticPyObject& obj);
    StaticPyObject& operator=(StaticPyObject&& obj) noexcept;

    /**
     * @brief Casts to the contained pybind object.
     *
     * @return The contained pybind object.
     */
    operator py::object() const;

    /**
     * @brief Gets a reference to the contained pybind object.
     * @note The reference is only valid for the lifetime of this object - but surely it's static
     *       right? :)
     * @note GIL safe.
     *
     * @return A reference to contained pybind object.
     */
    py::object& obj(void);

    /**
     * @brief Converts this object to another pybind object, borrowing the reference.
     *
     * @tparam T The type to cast to.
     * @return The new pybind object.
     */
    template <typename T = py::object>
    T borrow(void) {
        return py::reinterpret_borrow<T>(this->inner_obj);
    }

    /**
     * @brief Checks if the underlying object is valid.
     * @note GIL safe.
     *
     * @return True if the underlying object is not null.
     */
    explicit operator bool() const;

    /**
     * @brief Forwards to the underlying object's call operator.
     *
     * @tparam policy Return value policy to formward.
     * @tparam Args Types of the args to forward.
     * @param args Args to forward.
     * @return The return value of the function.
     */
    template <py::return_value_policy policy = py::return_value_policy::automatic_reference,
              typename... Args>
    py::object operator()(Args&&... args) const {
        return this->inner_obj.template operator()<policy, Args...>(std::forward<Args>(args)...);
    }
};

}  // namespace pyunrealsdk

#endif /* PYUNREALSDK_STATIC_PY_OBJECT_H */
