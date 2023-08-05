#ifndef PYUNREALSDK_EXPORTS_H
#define PYUNREALSDK_EXPORTS_H

/*
Helper macros to deal with exported functions.

User code should never need to use these, they should always be hidden in implementation files.
*/

/**
 * @fn PYUNREALSDK_MANGLE
 * @brief Mangles an exported symbol name.
 *
 * @param name The name to mangle.
 * @return The mangled name
 */

/**
 * @fn PYUNREALSDK_CAPI
 * @brief Declares an exported C function.
 * @note The exported function is automatically mangled, use `UNREALSDK_MANGLE()` when calling it.
 *
 * @param ret The return type. May include attributes.
 * @param name The function name.
 * @param ... The function args (including types).
 * @returns A function declaration. May be left as a forward declaration, or used in the definition.
 */

// =================================================================================================

// Unconditionally mangle - if we disabled this when not shared we might cause name conflicts (e.g.
// if only the return type differs)
#define PYUNREALSDK_MANGLE(name) _pyunrealsdk_export__##name

// Determine the correct dllimport/export attribute
#if defined(PYUNREALSDK_INTERNAL)

#if defined(__clang__) || defined(__MINGW32__)
#define PYUNREALSDK_DLLEXPORT [[gnu::dllexport]]
#elif defined(_MSC_VER)
#define PYUNREALSDK_DLLEXPORT __declspec(dllexport)
#else
#error Unknown dllexport attribute
#endif

#else  //  defined(PYUNREALSDK_INTERNAL)

#if defined(__clang__) || defined(__MINGW32__)
#define PYUNREALSDK_DLLEXPORT [[gnu::dllimport]]
#elif defined(_MSC_VER)
#define PYUNREALSDK_DLLEXPORT __declspec(dllimport)
#else
#error Unknown dllimport attribute
#endif

#endif  //  defined(PYUNREALSDK_INTERNAL)

// Need extern C to create a valid export
// Use the relevant dllimport/export attribute
// Mangle the function name to avoid conflicts
// MSVC needs noexpect(false) to allow exceptions through the C interface - since it's standard C++,
//  might as well just add across all compilers
#define PYUNREALSDK_CAPI(ret, name, ...) \
    extern "C" PYUNREALSDK_DLLEXPORT ret PYUNREALSDK_MANGLE(name)(__VA_ARGS__) noexcept(false)

#endif /* PYUNREALSDK_EXPORTS_H */
