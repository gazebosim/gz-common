
# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable
ign_get_libsources_and_unittests(sources gtest_sources)

# Create the library target
ign_create_core_library(
  SOURCES ${sources}
  CXX_STANDARD 11)

# Link the libraries that we always need
target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PRIVATE ${DL_TARGET})

# Handle non-Windows configuration settings
if(NOT WIN32)

  # Link the libraries that we don't expect to find on Windows
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
    PUBLIC UUID::UUID
    pthread)

endif()

# don't build MovingWindowFilter_TEST if we don't have ignition-math
if(NOT ignition-math${IGN_MATH_VER}_FOUND)
  list(REMOVE_ITEM gtest_sources MovingWindowFilter_TEST.cc)
endif()

# Build the unit tests
ign_build_tests(
  TYPE UNIT
  SOURCES ${gtest_sources}
  INCLUDE_DIRS
    # Used to make internal source file headers visible to the unit tests
    ${CMAKE_CURRENT_SOURCE_DIR}
    # Used to make test-directory headers visible to the unit tests
    ${PROJECT_SOURCE_DIR}
    # Used to make test_config.h visible to the unit tests
    ${PROJECT_BINARY_DIR})

if(TARGET UNIT_MovingWindowFilter_TEST)
  target_include_directories(UNIT_MovingWindowFilter_TEST PRIVATE
    ${ignition-math${IGN_MATH_VER}_INCLUDE_DIRS})
endif()

if(TARGET UNIT_PluginLoader_TEST)
  target_compile_definitions(UNIT_PluginLoader_TEST PRIVATE
    "IGN_COMMON_LIB_PATH=\"$<TARGET_FILE_DIR:${PROJECT_LIBRARY_TARGET_NAME}>\"")
endif()

# Produce warning on Windows if the user has decided to turn on the symlink
# tests. In order for those tests to work, they will need to run the tests in
# administrative mode, or use some other workaround.
if(WIN32)
  if(IGN_BUILD_SYMLINK_TESTS_ON_WINDOWS)
    message(STATUS "")
    message(STATUS "You have opted to enable symlink tests on a Windows platform.")
    message(STATUS "The test UNIT_Filesystem_TEST will require elevated privileges")
    message(STATUS "in order to succeed. For more information, see the issue")
    message(STATUS "https://bitbucket.org/ignitionrobotics/ign-common/issues/21")
    message(STATUS "")
    target_compile_definitions(UNIT_Filesystem_TEST PRIVATE IGN_BUILD_SYMLINK_TESTS_ON_WINDOWS)
  endif()
endif()
