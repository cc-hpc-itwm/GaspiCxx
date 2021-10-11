find_package(PythonModules REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/pygpi.cpp
    )

set(PYLIB_MODULE_NAME "pygpi")
pybind11_add_module(${PYLIB_MODULE_NAME} MODULE
                    ${SOURCE_FILES})
#pybind11_strip(${PYLIB_MODULE_NAME})
target_link_libraries(${PYLIB_MODULE_NAME} PRIVATE pybind11::module
                                                   GaspiCxx)

install(TARGETS ${PYLIB_MODULE_NAME}
        EXPORT ${PYLIB_MODULE_NAME}Config
        ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(EXPORT ${PYLIB_MODULE_NAME}Config
        FILE
          ${PYLIB_MODULE_NAME}Config.cmake
        NAMESPACE
          ${PYLIB_MODULE_NAME}::
        DESTINATION
          ${CMAKE_INSTALL_LIBDIR}/cmake)

