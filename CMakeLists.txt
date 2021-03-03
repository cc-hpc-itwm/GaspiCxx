cmake_minimum_required (VERSION 3.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project (GaspiCxx)

include(GNUInstallDirs)
find_package(GPI2 REQUIRED)

option (ENABLE_TESTS "Compile tests [default: disabled]" off)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")

set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

add_subdirectory (src)
add_subdirectory (examples)
if(ENABLE_TESTS)
  add_subdirectory (tests)
endif()
