add_library(_pyunrealsdk_python INTERFACE)

target_include_directories(_pyunrealsdk_python INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/${UNREALSDK_ARCH}/include"
)

# Python debug files are all formatted `[name]_d.[extension]`
# While we can easily glob for them, it makes it annoying to find the release versions
# Instead, we glob for everything, then do a regex match to filter down
# We'll repeat this pattern a few more times

file(GLOB _pyunrealsdk_python_libs "${CMAKE_CURRENT_SOURCE_DIR}/${UNREALSDK_ARCH}/libs/*.lib")
foreach(_file ${_pyunrealsdk_python_libs})
    if ("${_file}" MATCHES "_d\.[A-Za-z]+$")
        target_link_libraries(_pyunrealsdk_python INTERFACE "$<$<CONFIG:DEBUG>:${_file}>")
    else()
        target_link_libraries(_pyunrealsdk_python INTERFACE "$<$<NOT:$<CONFIG:DEBUG>>:${_file}>")
    endif()
endforeach()

# We need to install some files into the root dir, and some into /DLLs
# Can't seem to easily make install(FILES ...) preserve dir, so function it is

function(_install_python_files source_folder install_folder)
    file(GLOB dlls "${source_folder}/*.dll")
    file(GLOB pyds "${source_folder}/*.pyd")
    file(GLOB zips "${source_folder}/*.zip")
    foreach(_file ${dlls} ${pyds} ${zips})
        if ("${_file}" MATCHES "_d\.[A-Za-z]+$")
            install(
                FILES ${_file}
                CONFIGURATIONS Debug
                DESTINATION ${install_folder}
            )
        else()
            install(
                FILES ${_file}
                CONFIGURATIONS Release
                DESTINATION ${install_folder}
            )
        endif()
    endforeach()

    # PDBs are always debug
    file(GLOB_RECURSE pdbs "${source_folder}/*.pdb")
    install(
        FILES ${pdbs}
        CONFIGURATIONS Debug
        DESTINATION ${install_folder}
    )
endfunction()

_install_python_files("${CMAKE_CURRENT_SOURCE_DIR}/${UNREALSDK_ARCH}" ".")
_install_python_files("${CMAKE_CURRENT_SOURCE_DIR}/${UNREALSDK_ARCH}/DLLs" "DLLs")
