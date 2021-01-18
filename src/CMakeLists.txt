# Define library. Only source files here!
project(libGaspiCxx LANGUAGES CXX)

#Create a library called "Hello" which includes the source file "hello.icxx".
# The extension is already found. Any number of sources could be listed here.
add_library ( GaspiCxx
              Context.cpp 
	      Runtime.cpp
	      collectives/Allgather.cpp
	      collectives/Allreduce.cpp
	      collectives/Alltoall.cpp
	      group/Group.cpp
	      group/Rank.cpp
	      passive/Passive.cpp
	      segment/Segment.cpp
              segment/SegmentManager.cpp
              segment/SegmentResource.cpp
              segment/MemoryManager.cpp
              segment/NotificationManager.cpp
	      singlesided/Buffer.cpp
              singlesided/BufferDescription.cpp
	      singlesided/Endpoint.cpp
              singlesided/Queue.cpp
              singlesided/write/SourceBuffer.cpp
              singlesided/write/TargetBuffer.cpp
	      utility/Filesystem.cpp
              utility/LockGuard.cpp
              utility/serialization.cpp
	      )

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(GaspiCxx 
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
   	       $<INSTALL_INTERFACE:include>
   PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GaspiCxx
	GPI2::GPI2
	Threads::Threads)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS GaspiCxx EXPORT GaspiCxxConfig
	ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the build directory
export(TARGETS GaspiCxx FILE GaspiCxxConfig.cmake)
