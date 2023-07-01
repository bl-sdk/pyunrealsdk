add_library(_pyunrealsdk_python INTERFACE)

target_include_directories(_pyunrealsdk_python INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/${UNREALSDK_ARCH}/include"
)

file(GLOB _pyunrealsdk_python_libs "${CMAKE_CURRENT_SOURCE_DIR}/${UNREALSDK_ARCH}/libs/*.lib")
foreach(_lib ${_pyunrealsdk_python_libs})
    if ("${_lib}" MATCHES "_d.lib$")
        target_link_libraries(_pyunrealsdk_python INTERFACE "$<$<CONFIG:DEBUG>:${_lib}>")
    else()
        target_link_libraries(_pyunrealsdk_python INTERFACE "$<$<NOT:$<CONFIG:DEBUG>>:${_lib}>")
    endif()
endforeach()
