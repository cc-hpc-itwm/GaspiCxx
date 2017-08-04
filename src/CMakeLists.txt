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
	      singlesided/BufferDescription.cpp
	      singlesided/Queue.cpp
	      singlesided/write/SourceBuffer.cpp
	      singlesided/write/TargetBuffer.cpp
	      utility/LockGuard.cpp
	      utility/serialization.cpp
	      )

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
target_include_directories (GaspiCxx 
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include 
	       ${CMAKE_CURRENT_SOURCE_DIR} )
