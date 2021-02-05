enable_testing()
find_package(GTest REQUIRED)

add_executable ( GaspiCxxTests 
      run_tests.cpp
      GlobalTestConfiguration.cpp
      AllgatherTest.cpp
      AlltoallTest.cpp
      PassiveTest.cpp
      SegmentMemoryManagerTest.cpp 
      SingleSidedWriteBufferTest.cpp )

target_include_directories (GaspiCxxTests PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR} )	

target_link_libraries( GaspiCxxTests LINK_PUBLIC  
  	GTest::GTest 
	GTest::Main 
	GaspiCxx 
	GPI2::GPI2dbg
	Threads::Threads
	rt )

add_custom_target (run_tests
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.sh GaspiCxxTests 
	DEPENDS GaspiCxxTests 
	)
