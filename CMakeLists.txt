cmake_minimum_required (VERSION 3.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project (GaspiCxx)

include(GNUInstallDirs)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(GPI2 REQUIRED)

option (ENABLE_TESTS "Compile tests [default: disabled]" off)
option (BUILD_SHARED_LIBS "Create shared libraries [default: disabled (creates static libraries)]" off)
option (BUILD_PYTHON_BINDINGS "Build Python bindings [default: disabled]" off)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")

set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

add_subdirectory (src)
add_subdirectory (examples)

if(BUILD_PYTHON_BINDINGS)
  find_package(PythonModules REQUIRED)
  find_package(pybind11 CONFIG REQUIRED)
  add_subdirectory (src/python/bindings)
endif()

if(ENABLE_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  include(GoogleTest)
  add_subdirectory (tests)

  if(BUILD_PYTHON_BINDINGS)
    find_package(PythonModules COMPONENTS pytest)
    add_subdirectory (tests/python)
  endif()
endif()
