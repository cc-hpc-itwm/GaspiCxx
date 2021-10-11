include (parse_arguments)

function (gaspicxx_gen_environment_paths)
  set(multi_value_options VARIABLE_LIST)
  set(required_options VARIABLE_LIST)
  _parse_arguments(ARG "${options}" "${one_value_options}" 
                       "${multi_value_options}" "${required_options}" ${ARGN})
  set(env_var_names PATH LIBRARY_PATH LD_LIBRARY_PATH DYLD_LIBRARY_PATH CPATH PYTHONPATH)
  set(env_vars )

  foreach (var_name ${env_var_names})
    if (DEFINED ENV{${var_name}})
      list(APPEND env_vars "${var_name}=$ENV{${var_name}}")
    endif()
  endforeach()
  set(${ARG_VARIABLE_LIST} ${env_vars} PARENT_SCOPE)
endfunction()

function (gaspicxx_gen_executable_script)
  set(one_value_options SCRIPT_DIR SCRIPT_NAME)
  set(required_options SCRIPT_DIR SCRIPT_NAME)
  _parse_arguments(ARG "${options}" "${one_value_options}" 
                       "${multi_value_options}" "${required_options}" ${ARGN})

  set(tmp_script_path ${CMAKE_CURRENT_BINARY_DIR}/tmp/${ARG_SCRIPT_NAME})
  file(REMOVE ${ARG_SCRIPT_DIR}/${ARG_SCRIPT_NAME})
  file(WRITE ${tmp_script_path} "")
  file(COPY ${tmp_script_path} 
       DESTINATION ${ARG_SCRIPT_DIR}
       FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
       )
  file(REMOVE ${tmp_script_path})
endfunction()

function (gaspicxx_gen_gpi_machinefile)
  set(one_value_options NRANKS FILENAME)
  set(required_options NRANKS FILENAME)
  _parse_arguments(ARG "${options}" "${one_value_options}" 
                       "${multi_value_options}" "${required_options}" ${ARGN})

  file(WRITE ${ARG_FILENAME} "")
  cmake_host_system_information(RESULT hostname QUERY HOSTNAME)
  foreach(index RANGE 1 ${ARG_NRANKS})
    file(APPEND ${ARG_FILENAME} "${hostname}\n")
  endforeach()
endfunction()

function (gaspicxx_gen_test_script)
  set(one_value_options NAME SCRIPT_DIR TEST_EXECUTABLE)
  set(options IS_PYTHON_TEST)
  set(required_options NAME SCRIPT_DIR TEST_EXECUTABLE)
  _parse_arguments_with_unknown(ARG "${options}" "${one_value_options}" 
                                    "${multi_value_options}" "${required_options}" ${ARGN})
  
  message(STATUS "Test: Generating ${ARG_NAME} script")
  gaspicxx_gen_executable_script(SCRIPT_NAME ${ARG_NAME}
                                   SCRIPT_DIR ${ARG_SCRIPT_DIR})

  gaspicxx_gen_environment_paths(VARIABLE_LIST env_paths)

  set(script_path ${ARG_SCRIPT_DIR}/${ARG_NAME})
  foreach (var ${env_paths})
    file(APPEND ${script_path} "export ${var}\n")
  endforeach()
  if (ARG_IS_PYTHON_TEST)
    # Python test
    file(APPEND ${script_path} "export PYTHONPATH=${CMAKE_BINARY_DIR}:${CMAKE_BINARY_DIR}/src/python:${CMAKE_BINARY_DIR}/src/python/bindings:\$\{PYTHONPATH\}\n")
    file(APPEND ${script_path} "\n${Python_EXECUTABLE} -m pytest ${ARG_TEST_EXECUTABLE}\n")
  else()
    # regular executable test
    file(APPEND ${script_path} "\n${ARG_TEST_EXECUTABLE} $@\n")
  endif()
endfunction()

function (gaspicxx_add_gpi_test)
  set(one_value_options NAME TEST_SCRIPT NRANKS RUNCOMMAND
                        MACHINEFILE CLEANUP TIMEOUT SLEEP)
  set(multi_value_options LABELS)
  set(required_options NAME TEST_SCRIPT NRANKS RUNCOMMAND)
  _parse_arguments_with_unknown(ARG "${options}" "${one_value_options}"
                                    "${multi_value_options}" "${required_options}" ${ARGN})
  set(test_name ${ARG_NAME}_${ARG_NRANKS}ranks)

  # increase overall timeout time to include the sleep time after the actual test
  if (ARG_TIMEOUT)
    math(EXPR ARG_TIMEOUT "${ARG_SLEEP} + ${ARG_TIMEOUT}")
  endif()

  if (TEST_MACHINEFILE)
    set(ARG_MACHINEFILE ${TEST_MACHINEFILE})
    # get number of hostnames in the provided machinefile
    file(STRINGS "${ARG_MACHINEFILE}" HOSTS_LIST)
    list(LENGTH HOSTS_LIST CLUSTER_MAX_NUM_PROCS)
  endif()

  if (ARG_MACHINEFILE)
    # use user-defined machinefile
    set(runparams "-n ${ARG_NRANKS} -m ${ARG_MACHINEFILE}")
  else()
    # generate machinefile for ARG_NRANKS running on the localhost
    set(machinefile_path ${CMAKE_CURRENT_BINARY_DIR}/machinefile_${ARG_NAME}_${ARG_NRANKS}.tmp)
    gaspicxx_gen_gpi_machinefile(NRANKS ${ARG_NRANKS} 
                                   FILENAME ${machinefile_path})
    set(runparams "-n ${ARG_NRANKS} -m ${machinefile_path}")
  endif()

  set(CLEANUP_SCRIPT ${CMAKE_SOURCE_DIR}/cmake/cleanup.sh)

  if (NOT TEST_MACHINEFILE OR
      CLUSTER_MAX_NUM_PROCS GREATER_EQUAL ARG_NRANKS)
    # create gaspi_run test
    add_test(NAME ${test_name}
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMAND "${CMAKE_COMMAND}"
              -DRUNCOMMAND=${ARG_RUNCOMMAND}
              -DRUNCOMMAND_ARGS="${runparams}"
              -DTEST_SCRIPT="${ARG_TEST_SCRIPT}"
              -DTEST_ARGS="--gtest_filter=*${ARG_NAME}* --gtest_color=yes"
              -DTEST_DIR="${CMAKE_BINARY_DIR}"
              -DSLEEP="${ARG_SLEEP}"
              -DCLEANUP_SCRIPT="${CLEANUP_SCRIPT}"
              -P "${CMAKE_SOURCE_DIR}/cmake/run_test.cmake"
            ) 

    # set labels if specified
    if (ARG_LABELS)
      set_property(TEST ${test_name} PROPERTY LABELS ${ARG_LABELS})
    endif()

    # set cleanup fixture script if specified
    if (ARG_CLEANUP)
      set_tests_properties(${test_name} PROPERTIES FIXTURES_REQUIRED ${ARG_CLEANUP})
    endif()
    
    # set timeout if specified
    if (ARG_TIMEOUT)
      set_tests_properties(${test_name} PROPERTIES TIMEOUT ${ARG_TIMEOUT})
    endif()

    # make sure the GPI tests are not run in parallel 
    set_tests_properties(${test_name} PROPERTIES RESOURCE_LOCK GPI_run_serial)
  endif()
endfunction()


function (gaspicxx_generate_gpi_tests)
  set (one_value_options DESCRIPTION TIMEOUT TEST_EXECUTABLE)
  set (multi_value_options LOCALRANKS_LIST TEST_LIST)
  set (required_options LOCALRANKS_LIST TEST_LIST TEST_EXECUTABLE)
  _parse_arguments (ARG "${options}" "${one_value_options}" 
                        "${multi_value_options}" "${required_options}" ${ARGN})
  set(CLEANUP_TEST_NAME gpi_cleanup)

  # wrap call to the test executable in a script that exports the current environment
  # and can be executed with `gaspi_run`
  set(script_name "run_with_gpi.sh")
  set(script_path ${CMAKE_CURRENT_BINARY_DIR}/${script_name})
  gaspicxx_gen_test_script(NAME ${script_name}
                           SCRIPT_DIR ${CMAKE_CURRENT_BINARY_DIR}
                           TEST_EXECUTABLE ${ARG_TEST_EXECUTABLE})

  foreach(test_name ${ARG_TEST_LIST})
    message(STATUS "Test: Generating gaspi_run tests for ${test_name} with ${ARG_LOCALRANKS_LIST} ranks")
    foreach(nlocalranks ${ARG_LOCALRANKS_LIST})
      gaspicxx_add_gpi_test(NAME ${test_name}
                            NRANKS ${nlocalranks}
                            TEST_SCRIPT ${script_path}
                            RUNCOMMAND ${GPI2_GASPI_RUN}
                            CLEANUP ${CLEANUP_TEST_NAME}
                            TIMEOUT ${ARG_TIMEOUT}
                            SLEEP 0)
    endforeach()
  endforeach()
endfunction()


function (gaspicxx_generate_cleanup_test)
  set (one_value_options LOCALRANKS)
  set (required_options LOCALRANKS)
  _parse_arguments (ARG "${options}" "${one_value_options}" 
                        "${multi_value_options}" "${required_options}" ${ARGN})

  set(CLEANUP_TEST_NAME gpi_cleanup)
  set(CLEANUP_SCRIPT ${CMAKE_SOURCE_DIR}/cmake/cleanup.sh)

  # wrap call to the test executable in a script that exports the current environment
  # and can be executed with `gaspi_run`
  set(script_name "run_cleanup_with_gpi.sh")
  set(script_path ${CMAKE_CURRENT_BINARY_DIR}/${script_name})
  gaspicxx_gen_test_script(NAME ${script_name}
                           SCRIPT_DIR ${CMAKE_CURRENT_BINARY_DIR}
                           TEST_EXECUTABLE ${CLEANUP_SCRIPT})

  gaspicxx_add_gpi_test(NAME ${CLEANUP_TEST_NAME}
                        NRANKS ${ARG_LOCALRANKS}
                        TEST_SCRIPT ${script_path}
                        RUNCOMMAND ${GPI2_GASPI_RUN}
                        SLEEP 0)
  set_tests_properties(${CLEANUP_TEST_NAME}_${ARG_LOCALRANKS}ranks
                       PROPERTIES FIXTURES_CLEANUP ${CLEANUP_TEST_NAME})
endfunction()

function (gaspicxx_generate_python_gpi_test)
  set (one_value_options NAME TEST_EXECUTABLE DESCRIPTION TIMEOUT)
  set (multi_value_options LOCALRANKS_LIST LABELS ARGS)
  set (required_options NAME TEST_EXECUTABLE LOCALRANKS_LIST)
  _parse_arguments (ARG "${options}" "${one_value_options}" 
                        "${multi_value_options}" "${required_options}" ${ARGN})
  set(CLEANUP_TEST_NAME gpi_cleanup)
  list(APPEND ARG_LABELS "Python")
  list(REMOVE_DUPLICATES ARG_LABELS)

  set(ARG_NAME "Py_${ARG_NAME}")
  
  # wrap call to the test executable in a script that exports the current environment
  # the script can then be executed within a `gaspi_run` call
  set(script_name run_${ARG_NAME}.sh)
  set(script_path ${CMAKE_CURRENT_BINARY_DIR}/${script_name})
  gaspicxx_gen_test_script(NAME ${script_name}
                           SCRIPT_DIR ${CMAKE_CURRENT_BINARY_DIR}
                           TEST_EXECUTABLE ${ARG_TEST_EXECUTABLE}
                           IS_PYTHON_TEST)

  message(STATUS "Test: Generating gaspi_run tests for ${ARG_NAME} with ${ARG_LOCALRANKS_LIST} ranks")
  foreach(nlocalranks ${ARG_LOCALRANKS_LIST})
    gaspicxx_add_gpi_test(NAME ${ARG_NAME}
                          NRANKS ${nlocalranks}
                          TEST_SCRIPT ${script_path}
                          RUNCOMMAND ${GPI2_GASPI_RUN}
                          TIMEOUT ${ARG_TIMEOUT}
                          CLEANUP ${CLEANUP_TEST_NAME}
                          SLEEP 0
                          LABELS ${ARG_LABELS})
  endforeach()
endfunction()
