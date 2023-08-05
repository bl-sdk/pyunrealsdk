#ifndef PYUNREALSDK_PCH_H
#define PYUNREALSDK_PCH_H

#if defined(PYUNREALSDK_INTERNAL)

// If compiling internals, C api functions are exported
#if defined(__clang__) || defined(__MINGW32__)
#define PYUNREALSDK_CAPI extern "C" [[gnu::dllexport]]
#elif defined(_MSC_VER)
#define PYUNREALSDK_CAPI extern "C" __declspec(dllexport)
#else
#error Unknown dllexport attribute
#endif

// MSVC needs this to allow exceptions through the C interface
// Since it's standard C++, might as well just add it to everything
#define PYUNREALSDK_CAPI_SUFFIX noexcept(false)

#else

// If included as a library, we want to import from the internals
#if defined(__clang__) || defined(__MINGW32__)
#define PYUNREALSDK_CAPI extern "C" [[gnu::dllimport]]
#elif defined(_MSC_VER)
#define PYUNREALSDK_CAPI extern "C" __declspec(dllimport)
#else
#error Unknown dllimport attribute
#endif

#define PYUNREALSDK_CAPI_SUFFIX noexcept(false)

#endif

#include "unrealsdk/pch.h"

#ifdef __clang__
// Python, detecting `_MSC_VER`, tries to use `__int64`, which is an MSVC extension
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#endif

#include "Python.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __cplusplus

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// NOLINTNEXTLINE(misc-unused-alias-decls)
namespace py = pybind11;
using namespace pybind11::literals;

#include <variant>

// Type casters need to be defined the same way in every file, so best to put here
#include "pyunrealsdk/type_casters.h"

#endif

#endif /* PYUNREALSDK_PCH_H */
