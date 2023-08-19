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

set(GIT_PRE_CONFIGURE_FILE "src/pyunrealsdk/version.cpp.in")
set(GIT_POST_CONFIGURE_FILE "${CMAKE_CURRENT_BINARY_DIR}/version.cpp")
include(common_cmake/git_watcher.cmake)

set(PYBIND11_NOPYTHON True)
add_subdirectory(libs/pybind11 EXCLUDE_FROM_ALL)

set(UNREALSDK_SHARED True)
add_subdirectory(libs/unrealsdk EXCLUDE_FROM_ALL)

# Make sure to include after unrealsdk, to make sure the arch define is set
add_subdirectory(common_cmake/explicit_python)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/pyunrealsdk/*.cpp" "src/pyunrealsdk/*.h")
list(APPEND sources ${GIT_POST_CONFIGURE_FILE})
target_sources(pyunrealsdk PUBLIC ${sources})

target_include_directories(pyunrealsdk PUBLIC "src")
target_link_libraries(pyunrealsdk PUBLIC
    explicit_python
    unrealsdk
    pybind11::embed
    pybind11::windows_extras # Does nothing if not using MSVC
)

# Clang LTO has some issues if we also link this, so only enable on others
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_libraries(pyunrealsdk PUBLIC pybind11::lto)
endif()

target_compile_definitions(pyunrealsdk PUBLIC
    PYBIND11_SIMPLE_GIL_MANAGEMENT
    PYBIND11_DETAILED_ERROR_MESSAGES
)
target_compile_definitions(pyunrealsdk PRIVATE PYUNREALSDK_INTERNAL)

target_precompile_headers(pyunrealsdk PUBLIC "src/pyunrealsdk/pch.h")

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

install(
    TARGETS pyunrealsdk unrealsdk
    RUNTIME DESTINATION .
)
