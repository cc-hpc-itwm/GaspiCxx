cmake_minimum_required (VERSION 3.8)
project (GaspiCxx)

include(GNUInstallDirs)

set (Test on)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")

# add GPI2
FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(GPI2 REQUIRED GPI2>=1.3.0)
include_directories ( ${GPI2_INCLUDE_DIRS} )
link_directories ( ${GPI2_LIBRARY_DIRS} )

add_subdirectory (src)
add_subdirectory (examples)
if (Test)
  add_subdirectory (tests)
endif (Test)  
