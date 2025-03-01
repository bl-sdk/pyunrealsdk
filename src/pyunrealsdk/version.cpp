#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/version.h"

namespace pyunrealsdk {

PYUNREALSDK_CAPI([[nodiscard]] uint32_t, get_version);
#ifdef PYUNREALSDK_INTERNAL
PYUNREALSDK_CAPI([[nodiscard]] uint32_t, get_version) {
    // NOLINTNEXTLINE(readability-magic-numbers)
    return (VERSION_MAJOR & 0xFF) << 16 | (VERSION_MINOR & 0xFF) << 8 | (VERSION_PATCH & 0xFF);
}
#endif
[[nodiscard]] uint32_t get_version(void) {
    return PYUNREALSDK_MANGLE(get_version)();
}

namespace {
#ifdef PYUNREALSDK_INTERNAL

#include "pyunrealsdk/git.inl"

const constexpr auto GIT_HASH_CHARS = 8;
const std::string VERSION_STR = std::format("pyunrealsdk v{}.{}.{} ({}{})",
                                            VERSION_MAJOR,
                                            VERSION_MINOR,
                                            VERSION_PATCH,
                                            std::string(GIT_HEAD_SHA1).substr(0, GIT_HASH_CHARS),
                                            GIT_IS_DIRTY ? ", dirty" : "");
#endif
}  // namespace

PYUNREALSDK_CAPI([[nodiscard]] const char*, get_version_string);
#ifdef PYUNREALSDK_INTERNAL
PYUNREALSDK_CAPI([[nodiscard]] const char*, get_version_string) {
    return VERSION_STR.data();
}

[[nodiscard]] const std::string& get_version_string(void) {
    return VERSION_STR;
}
#else
[[nodiscard]] const std::string& get_version_string(void) {
    // NOLINTNEXTLINE(readability-identifier-naming)
    static const std::string VERSION_STR = PYUNREALSDK_MANGLE(get_version_string)();
    return VERSION_STR;
}
#endif

}  // namespace pyunrealsdk
