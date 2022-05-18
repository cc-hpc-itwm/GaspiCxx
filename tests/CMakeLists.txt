add_executable (GaspiCxxTests
                run_tests.cpp
                AllreduceNonBlockingTest.cpp
                AllreduceNonBlockingLowLevelTest.cpp
                AllgathervNonBlockingLowLevelTest.cpp
                AllgathervNonBlockingTest.cpp
                BroadcastNonBlockingTest.cpp
                BarrierTest.cpp
                RoundRobinDedicatedThreadTest.cpp
                PassiveTest.cpp
                SegmentMemoryManagerTest.cpp
                SingleSidedWriteBufferTest.cpp
)

target_include_directories(GaspiCxxTests
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(GaspiCxxTests
                      PUBLIC GTest::GTest
                             GaspiCxx)

add_custom_target (run_tests
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.sh GaspiCxxTests
	DEPENDS GaspiCxxTests)

include (test_helpers)
set(localranks_list 1 2 3 4)
set(test_list Allreduce
              Allgatherv
              Barrier
              Broadcast
              RoundRobinDedicatedThread
              Passive
              MemoryManager
              SingleSidedWriteBuffer
              )

gaspicxx_generate_gpi_tests(TEST_LIST "${test_list}"
                            LOCALRANKS_LIST "${localranks_list}"
                            TEST_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/GaspiCxxTests"
                            TIMEOUT 100)

gaspicxx_generate_cleanup_test(LOCALRANKS 4)
