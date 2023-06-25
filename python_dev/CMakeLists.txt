add_library(_pyunrealsdk_python INTERFACE)

target_include_directories(_pyunrealsdk_python INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/${UNREALSDK_ARCH}/include"
)

file(GLOB_RECURSE libs "${CMAKE_CURRENT_SOURCE_DIR}/${UNREALSDK_ARCH}/libs/*.lib")
target_link_libraries(_pyunrealsdk_python INTERFACE ${libs})
