enable_testing()
find_package(GTest REQUIRED)

add_executable ( run_tests
      run_tests.cpp
      GlobalTestConfiguration.cpp
      AllgatherTest.cpp
      AllreduceTest.cpp
      AlltoallTest.cpp
      PassiveTest.cpp
      SegmentMemoryManagerTest.cpp 
      SingleSidedWriteBufferTest.cpp )

target_include_directories (run_tests PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR} )	

target_link_libraries( run_tests LINK_PUBLIC  
  	GTest::GTest 
	GTest::Main 
	GaspiCxx 
	GPI2-dbg 
	pthread 
	rt )

#add_test(unittests ./run_tests.sh GASPI_PATH=${GPI2_LIBRARIES}/../)
add_custom_command(TARGET run_tests POST_BUILD COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.sh run_tests)
