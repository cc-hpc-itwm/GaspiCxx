cmake_minimum_required (VERSION 3.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project (GaspiCxx)

include(GNUInstallDirs)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(GPI2 REQUIRED)

option (ENABLE_TESTS "Compile tests [default: disabled]" off)
option (BUILD_SHARED_LIBS "Create shared libraries [default: disabled (creates static libraries)]" off)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")

set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

add_subdirectory (src)
add_subdirectory (src/python/bindings)
add_subdirectory (examples)
if(ENABLE_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  include(GoogleTest)
  add_subdirectory (tests)
endif()
