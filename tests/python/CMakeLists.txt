include (test_helpers)

set(localranks_list 1 5 7)
gaspicxx_generate_python_gpi_test(NAME Group
                                  LOCALRANKS_LIST "${localranks_list}"
                                  TEST_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/group.py
                                  TIMEOUT 60)

set(localranks_list 2)
gaspicxx_generate_python_gpi_test(NAME Broadcast
                                  LOCALRANKS_LIST "${localranks_list}"
                                  TEST_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/broadcast.py
                                  TIMEOUT 60)
set(localranks_list 2)
gaspicxx_generate_python_gpi_test(NAME Allreduce
                                  LOCALRANKS_LIST "${localranks_list}"
                                  TEST_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/allreduce.py
                                  TIMEOUT 60)
