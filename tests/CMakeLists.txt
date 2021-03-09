add_executable (GaspiCxxTests
                run_tests.cpp
                AllgatherTest.cpp
                AllreduceNonBlockingTest.cpp
                AllreduceNonBlockingLowLevelTest.cpp
                BroadcastNonBlockingTest.cpp
                AlltoallTest.cpp
                BarrierTest.cpp
                RoundRobinDedicatedThreadTest.cpp
                PassiveTest.cpp
                SegmentMemoryManagerTest.cpp
                SingleSidedWriteBufferTest.cpp)

target_include_directories(GaspiCxxTests
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(GaspiCxxTests
                      PUBLIC GTest::GTest
                             GaspiCxx)

add_custom_target (run_tests
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.sh GaspiCxxTests
	DEPENDS GaspiCxxTests)

