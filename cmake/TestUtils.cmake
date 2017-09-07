#################################################
macro (ign_build_tests)
  # Find the Python interpreter for running the
  # check_test_ran.py script
  find_package(PythonInterp QUIET)

  # Build all the tests
  foreach(GTEST_SOURCE_file ${ARGN})
    string(REGEX REPLACE ".cc" "" BINARY_NAME ${GTEST_SOURCE_file})
    set(BINARY_NAME ${TEST_TYPE}_${BINARY_NAME})
    if(USE_LOW_MEMORY_TESTS)
      add_definitions(-DUSE_LOW_MEMORY_TESTS=1)
    endif(USE_LOW_MEMORY_TESTS)

    if (BINARY_NAME STREQUAL "UNIT_ColladaExporter_TEST")
      list (APPEND GTEST_SOURCE_file ${CMAKE_SOURCE_DIR}/src/tinyxml2/tinyxml2.cpp)
    endif()

    add_executable(${BINARY_NAME} ${GTEST_SOURCE_file})

    add_test(${BINARY_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}
             --gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/${BINARY_NAME}.xml)

    target_link_libraries(${BINARY_NAME}
        gtest
        gtest_main
        ${PROJECT_NAME_LOWER}.lib
        ${PROJECT_NAME_LOWER}
        ${IGNITION-MATH_LIBRARIES})

     if(WIN32)
       # If we have not installed our project's library yet, then it will not be visible
       # to the test when we attempt to run it. Therefore, we place a copy of our project's 
       # library into the directory that contains the test executable. We do not need to do
       # this for any of the test's other dependencies, because the fact that they were found
       # by the build system means they are installed and will be visible when the test is run.

       # Get the full file path to the original dll for this project
       set(dll_original "$<TARGET_FILE:${PROJECT_LIBRARY_TARGET_NAME}>")

       # Get the full file path for where we need to paste the dll for this project
       set(dll_target "$<TARGET_FILE_DIR:${BINARY_NAME}>/$<TARGET_FILE_NAME:${PROJECT_LIBRARY_TARGET_NAME}>")
       
       # Add the copy_if_different command as a custom command that is tied the target
       # of this test.
       add_custom_command(
         TARGET ${BINARY_NAME}
         COMMAND ${CMAKE_COMMAND}
         ARGS -E copy_if_different ${dll_original} ${dll_target}
         VERBATIM)

     endif(WIN32)

    set_tests_properties(${BINARY_NAME} PROPERTIES TIMEOUT 240)

    if(PYTHONINTERP_FOUND)
      # Check that the test produced a result and create a failure if it didn't.
      # Guards against crashed and timed out tests.
      add_test(check_${BINARY_NAME} ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/check_test_ran.py
        ${CMAKE_BINARY_DIR}/test_results/${BINARY_NAME}.xml)
    endif()
  endforeach()
endmacro()
