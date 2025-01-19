cmake_minimum_required(VERSION 3.25)

project(pyunrealsdk VERSION 1.6.0)

function(_pyunrealsdk_add_base_target_args target_name)
    target_compile_features(${target_name} PUBLIC cxx_std_20)
    set_target_properties(${target_name} PROPERTIES
        COMPILE_WARNING_AS_ERROR True
        INTERPROCEDURAL_OPTIMIZATION True
        EXPORT_COMPILE_COMMANDS True
        PREFIX ""
    )
    if(MSVC)
        # Under MSVC, enable edit and continue in debug - which conflicts with LTO
        set_target_properties(${target_name} PROPERTIES
            MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:EditAndContinue>"
            INTERPROCEDURAL_OPTIMIZATION $<CONFIG:Release>
        )
    elseif(MINGW)
        # Under MinGW, only enable LTO in release mode - it causes linking errors in debug
        set_target_properties(${target_name} PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION $<CONFIG:Release>
        )
    endif()

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    # CMake doesn't understand warnings as errors for MinGW yet
    if(MINGW)
        target_compile_options(${target_name} PRIVATE -Werror)
    endif()
endfunction()

add_library(pyunrealsdk SHARED)
_pyunrealsdk_add_base_target_args(pyunrealsdk)

set(CONFIGURE_FILES_DIR "${CMAKE_CURRENT_BINARY_DIR}/configure")

configure_file(
    "src/pyunrealsdk/version.inl.in"
    "${CONFIGURE_FILES_DIR}/pyunrealsdk/version.inl"
)

set(GIT_PRE_CONFIGURE_FILE "src/pyunrealsdk/git.inl.in")
set(GIT_POST_CONFIGURE_FILE "${CONFIGURE_FILES_DIR}/pyunrealsdk/git.inl")
include(common_cmake/git_watcher.cmake)

set(PYBIND11_NOPYTHON True)
add_subdirectory(libs/pybind11 EXCLUDE_FROM_ALL)

set(UNREALSDK_SHARED True)
add_subdirectory(libs/unrealsdk EXCLUDE_FROM_ALL)

# Make sure to include after unrealsdk, to make sure the arch define is set
add_subdirectory(common_cmake/explicit_python)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/pyunrealsdk/*.cpp" "src/pyunrealsdk/*.h")
target_sources(pyunrealsdk PUBLIC
    ${sources}
    ${GIT_POST_CONFIGURE_FILE}
    "src/pyunrealsdk/versioninfo.rc"
)

target_include_directories(pyunrealsdk PUBLIC "src" ${CONFIGURE_FILES_DIR})
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

function(pyunrealsdk_add_module target_name)
    add_library(${target_name} MODULE ${ARGN})
    _pyunrealsdk_add_base_target_args(${target_name})

    set_target_properties(${target_name} PROPERTIES
        DEBUG_POSTFIX "_d"
        SUFFIX ".pyd"
    )

    target_link_libraries(${target_name} PUBLIC pyunrealsdk)
endfunction()
