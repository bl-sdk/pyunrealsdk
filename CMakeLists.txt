cmake_minimum_required(VERSION 3.24)

project(pyunrealsdk)

add_library(pyunrealsdk SHARED)

target_compile_features(pyunrealsdk INTERFACE cxx_std_20)
set_target_properties(pyunrealsdk PROPERTIES
    COMPILE_WARNING_AS_ERROR True
    INTERPROCEDURAL_OPTIMIZATION True
    EXPORT_COMPILE_COMMANDS True
    PREFIX ""
)
if(MSVC)
    target_compile_options(pyunrealsdk PRIVATE /W4)
else()
    target_compile_options(pyunrealsdk PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_subdirectory(python_dev)

set(PYBIND11_NOPYTHON True)
add_subdirectory(libs/pybind11)

set(UNREALSDK_SHARED True)
add_subdirectory(libs/unrealsdk)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/pyunrealsdk/*.cpp" "src/pyunrealsdk/*.h")
target_sources(pyunrealsdk PRIVATE ${sources})

target_include_directories(pyunrealsdk PUBLIC "src")
target_link_libraries(pyunrealsdk PUBLIC
    _pyunrealsdk_python
    unrealsdk
    pybind11::embed
    pybind11::lto
)
if(MSVC)
	target_link_libraries(unrealsdk PUBLIC pybind11::windows_extras)
endif()

target_precompile_headers(pyunrealsdk INTERFACE "src/pyunrealsdk/pch.h")

if(MSVC)
    # Enable Edit and Continue - replace /Zi with /ZI
    string(REPLACE "/Zi" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    target_compile_options(pyunrealsdk INTERFACE "$<$<CONFIG:DEBUG>:/ZI>")

    target_link_options(pyunrealsdk INTERFACE "/INCREMENTAL")

    # Only enable /GL (which conflicts with /ZI) in release mode
    string(REPLACE "/GL" "" CMAKE_C_COMPILE_OPTIONS_IPO "${CMAKE_C_COMPILE_OPTIONS_IPO}")
    string(REPLACE "/GL" "" CMAKE_CXX_COMPILE_OPTIONS_IPO "${CMAKE_CXX_COMPILE_OPTIONS_IPO}")
    target_compile_options(pyunrealsdk INTERFACE "$<$<CONFIG:RELEASE>:/GL>")

    # UTF-8 encoded source files
    target_compile_options(pyunrealsdk PUBLIC "/utf-8")
endif()


if(MINGW)
    # Link `libc++.dll` and `libunwind.dll` statically
    target_link_options(pyunrealsdk PRIVATE "-static-libstdc++" "-Wl,-Bstatic" "-lunwind")
endif()

# Postbuild
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(POSTBUILD_SCRIPT "postbuild")
    if(CMAKE_HOST_WIN32)
        set(POSTBUILD_SCRIPT "${POSTBUILD_SCRIPT}.bat")
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${POSTBUILD_SCRIPT}")
        add_custom_command(
            TARGET pyunrealsdk
            POST_BUILD
            COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/${POSTBUILD_SCRIPT}"
            ARGS
                "$<SHELL_PATH:$<TARGET_FILE:pyunrealsdk>>"
                "$<SHELL_PATH:$<TARGET_FILE:unrealsdk>>"
                "${UNREALSDK_UE_VERSION}"
                "${UNREALSDK_ARCH}"
                "$<IF:$<CONFIG:DEBUG>,DEBUG,RELEASE>"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
endif()
