cmake_minimum_required (VERSION 3.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project (GaspiCxx)

include(GNUInstallDirs)

set (Test on)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")

# add GPI2
find_package(GPI2)

add_subdirectory (src)
add_subdirectory (examples)
if (Test)
  add_subdirectory (tests)
endif (Test)  
