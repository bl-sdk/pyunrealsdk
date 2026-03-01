#ifndef PYUNREALSDK_PCH_H
#define PYUNREALSDK_PCH_H

// Include the C exports library first, so we can use it everywhere
// This file is purely macros, it doesn't rely on anything else
#include "pyunrealsdk/exports.h"

// Inherit all of unrealsdk's PCH
#include "unrealsdk/pch.h"

#ifdef __clang__
// Python, detecting `_MSC_VER`, tries to use `__int64`, which is an MSVC extension
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#endif

// Expect to be linking against the release Python, even if we're building debug pyunrealsdk
#if defined(_DEBUG) && !defined(PYUNREALSDK_LINK_AGAINST_DEBUG_PYTHON)
#define _PYUNREALSDK_DEBUG
#undef _DEBUG
#endif

#include "Python.h"

#ifdef _PYUNREALSDK_DEBUG
#undef _PYUNREALSDK_DEBUG
#define _DEBUG
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __cplusplus

#include <pybind11/embed.h>
#include <pybind11/native_enum.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/warnings.h>

// NOLINTNEXTLINE(misc-unused-alias-decls)
namespace py = pybind11;
using namespace pybind11::literals;

// By default, pybind tries to compile with visibility hidden. This macro copies it, to avoid
// warnings when we have a type holding/inheriting from python objects, due to our types having
// greater visibility.
#if defined(__MINGW32__)
#define PY_OBJECT_VISIBILITY __attribute__((visibility("hidden")))
#else
#define PY_OBJECT_VISIBILITY
#endif

// Type casters need to be defined the same way in every file, so best to put here
#include "pyunrealsdk/type_casters.h"

#endif

#endif /* PYUNREALSDK_PCH_H */
