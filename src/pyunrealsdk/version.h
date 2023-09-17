#ifndef PYUNREALSDK_VERSION_H
#define PYUNREALSDK_VERSION_H

#include "pyunrealsdk/pch.h"

namespace pyunrealsdk {

#ifdef PYUNREALSDK_INTERNAL

#include "pyunrealsdk/version.inl"
inline constexpr auto VERSION_MAJOR = PROJECT_VERSION_MAJOR;
inline constexpr auto VERSION_MINOR = PROJECT_VERSION_MINOR;
inline constexpr auto VERSION_PATCH = PROJECT_VERSION_PATCH;

#undef PROJECT_VERSION_MAJOR
#undef PROJECT_VERSION_MINOR
#undef PROJECT_VERSION_PATCH

#endif

/**
 * @brief Gets the version number of the python sdk.
 * @note Packs as `(major & 0xFF) << 16 | (minor & 0xFF) << 8 | (patch & 0xFF)`.
 *
 * @return The packed version number the python sdk was compiled with.
 */
[[nodiscard]] uint32_t get_version(void);

/**
 * @brief Get the python sdk version as a printable string.
 *
 * @return The python sdk version string.
 */
[[nodiscard]] const std::string& get_version_string(void);

}  // namespace pyunrealsdk

#endif /* PYUNREALSDK_VERSION_H */
