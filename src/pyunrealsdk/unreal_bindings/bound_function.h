#ifndef PYUNREALSDK_UNREAL_BINDINGS_BOUND_FUNCTION_H
#define PYUNREALSDK_UNREAL_BINDINGS_BOUND_FUNCTION_H

#include "pyunrealsdk/pch.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

#ifdef PYUNREALSDK_INTERNAL

namespace unrealsdk::unreal {

class UFunction;
class UProperty;

}  // namespace unrealsdk::unreal

namespace pyunrealsdk::unreal {

namespace impl {

// Type helping convert a python function call to an unreal one.
struct PyCallInfo {
    unrealsdk::unreal::WrappedStruct params;
    unrealsdk::unreal::UProperty* return_param{};
    std::vector<unrealsdk::unreal::UProperty*> out_params;

    /**
     * @brief Converts python args into a params struct.
     *
     * @param func The function being called.
     * @param args The python args.
     * @param kwargs The python kwargs.
     */
    PyCallInfo(const unrealsdk::unreal::UFunction* func,
               const py::args& args,
               const py::kwargs& kwargs);

    /**
     * @brief Get the python return value for the function call from the contained params struct.
     *
     * @return The value to return to python.
     */
    [[nodiscard]] py::object get_py_return(void) const;
};

}  // namespace impl

/**
 * @brief Registers BoundFunction.
 *
 * @param module The module to register within.
 */
void register_bound_function(py::module_& mod);

}  // namespace pyunrealsdk::unreal

#endif

#endif /* PYUNREALSDK_UNREAL_BINDINGS_BOUND_FUNCTION_H */
