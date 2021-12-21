add_subdirectory(bindings)

# Python code
set(PYTHON_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/pygpi)

add_custom_target(CopyPythonFiles ALL
    DEPENDS ${PYTHON_DIRS}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PYTHON_DIRS} ${CMAKE_BINARY_DIR}/python/pygpi)

install(DIRECTORY ${PYTHON_DIRS}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FILES_MATCHING PATTERN "*.py")
