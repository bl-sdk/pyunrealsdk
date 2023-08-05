#ifndef PYUNREALSDK_VERSION_H
#define PYUNREALSDK_VERSION_H

#include "pyunrealsdk/pch.h"

#ifdef PYUNREALSDK_INTERNAL

namespace pyunrealsdk {

inline constexpr auto VERSION_MAJOR = 1;
inline constexpr auto VERSION_MINOR = 0;
inline constexpr auto VERSION_PATCH = 0;

/**
 * @brief Get the python sdk version as a printable string.
 *
 * @return The sdk version string.
 */
const std::string& get_version_string(void);

}  // namespace pyunrealsdk

#endif

#endif /* PYUNREALSDK_VERSION_H */
